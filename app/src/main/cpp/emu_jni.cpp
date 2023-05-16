// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <csignal>
#include <pthread.h>
#include <android/asset_manager_jni.h>
#include <sys/system_properties.h>
#include "strato/common.h"
#include "strato/common/language.h"
#include "strato/common/signal.h"
#include "strato/common/android_settings.h"
#include "strato/common/trace.h"
#include "strato/loader/loader.h"
#include "strato/vfs/android_asset_filesystem.h"
#include "strato/os.h"
#include "strato/jvm.h"
#include "strato/gpu.h"
#include "strato/audio.h"
#include "strato/input.h"
#include "strato/kernel/types/KProcess.h"
#include "strato/logger/logger.h"

jint Fps; //!< An approximation of the amount of frames being submitted every second
jfloat AverageFrametimeMs; //!< The average time it takes for a frame to be rendered and presented in milliseconds
jfloat AverageFrametimeDeviationMs; //!< The average deviation of the average frametimes in milliseconds

std::weak_ptr<strato::kernel::OS> OsWeak;
std::weak_ptr<strato::gpu::GPU> GpuWeak;
std::weak_ptr<strato::audio::Audio> AudioWeak;
std::weak_ptr<strato::input::Input> InputWeak;
std::weak_ptr<strato::Settings> SettingsWeak;

// https://cs.android.com/android/platform/superproject/+/master:bionic/libc/tzcode/bionic.cpp;l=43;drc=master;bpv=1;bpt=1
static std::string GetTimeZoneName() {
    const char *nameEnv = getenv("TZ");
    if (nameEnv)
        return std::string(nameEnv);

    char propBuf[PROP_VALUE_MAX];
    if (__system_property_get("persist.sys.timezone", propBuf)) {
        std::string nameProp(propBuf);

        // Flip -/+, see https://cs.android.com/android/platform/superproject/+/master:bionic/libc/tzcode/bionic.cpp;l=53;drc=master;bpv=1;bpt=1
        if (nameProp.size() > 2) {
            if (nameProp[2] == '-')
                nameProp[2] = '+';
            else if (nameProp[2] == '+')
                nameProp[2] = '-';
        }

        return nameProp;
    }

    // Fallback to GMT
    return "GMT";
}

extern "C" JNIEXPORT void Java_emu_strato_EmulationActivity_executeApplication(
    JNIEnv *env,
    jobject instance,
    jstring romUriJstring,
    jint romType,
    jint romFd,
    jobject settingsInstance,
    jstring publicAppFilesPathJstring,
    jstring privateAppFilesPathJstring,
    jstring nativeLibraryPathJstring,
    jobject assetManager
) {
    strato::signal::ScopedStackBlocker stackBlocker; // We do not want anything to unwind past JNI code as there are invalid stack frames which can lead to a segmentation fault
    Fps = 0;
    AverageFrametimeMs = AverageFrametimeDeviationMs = 0.0f;

    pthread_setname_np(pthread_self(), "EmuMain");

    auto jvmManager{std::make_shared<strato::JvmManager>(env, instance)};

    std::shared_ptr<strato::Settings> settings{std::make_shared<strato::AndroidSettings>(env, settingsInstance)};

    strato::JniString publicAppFilesPath(env, publicAppFilesPathJstring);

    strato::AsyncLogger::Initialize(*settings->logLevel, publicAppFilesPath + "logs/emulation.log");

    auto start{std::chrono::steady_clock::now()};

    // Initialize tracing
    perfetto::TracingInitArgs args;
    args.backends |= perfetto::kSystemBackend;
    args.shmem_size_hint_kb = 0x200000;
    perfetto::Tracing::Initialize(args);
    perfetto::TrackEvent::Register();

    try {
        strato::JniString nativeLibraryPath(env, nativeLibraryPathJstring);
        strato::JniString privateAppFilesPath{env, privateAppFilesPathJstring};

        auto os{std::make_shared<strato::kernel::OS>(
            jvmManager,
            settings,
            publicAppFilesPath,
            privateAppFilesPath,
            nativeLibraryPath,
            GetTimeZoneName(),
            std::make_shared<strato::vfs::AndroidAssetFileSystem>(AAssetManager_fromJava(env, assetManager))
        )};
        OsWeak = os;
        GpuWeak = os->state.gpu;
        AudioWeak = os->state.audio;
        InputWeak = os->state.input;
        SettingsWeak = settings;
        jvmManager->InitializeControllers();

        LOGDNF("Launching ROM {}", strato::JniString(env, romUriJstring));

        os->Execute(romFd, static_cast<strato::loader::RomFormat>(romType));
    } catch (std::exception &e) {
        LOGENF("An uncaught exception has occurred: {}", e.what());
    } catch (const strato::signal::SignalException &e) {
        LOGENF("An uncaught signal exception has occurred: {}", e.what());
    } catch (...) {
        LOGENF("An unknown uncaught exception has occurred");
    }

    perfetto::TrackEvent::Flush();

    InputWeak.reset();

    auto end{std::chrono::steady_clock::now()};
    LOGINF("Emulation has ended in {}ms", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

    strato::AsyncLogger::Finalize(true);
    close(romFd);
}

extern "C" JNIEXPORT jboolean Java_emu_strato_EmulationActivity_stopEmulation(JNIEnv *, jobject, jboolean join) {
    auto os{OsWeak.lock()};
    if (!os)
        return false;
    auto process{os->state.process};
    if (!process)
        return false;
    process->Kill(join, false, true);
    return true;
}

extern "C" JNIEXPORT jboolean Java_emu_strato_EmulationActivity_setSurface(JNIEnv *, jobject, jobject surface) {
    auto gpu{GpuWeak.lock()};
    if (!gpu)
        return false;
    gpu->presentation.UpdateSurface(surface);
    return true;
}

extern "C" JNIEXPORT void Java_emu_strato_EmulationActivity_changeAudioStatus(JNIEnv *, jobject, jboolean play) {
    auto audio{AudioWeak.lock()};
    if (audio)
        if (play)
            audio->Resume();
        else
            audio->Pause();
}

extern "C" JNIEXPORT void Java_emu_strato_EmulationActivity_updatePerformanceStatistics(JNIEnv *env, jobject thiz) {
    static jclass clazz{};
    if (!clazz)
        clazz = env->GetObjectClass(thiz);

    static jfieldID fpsField{};
    if (!fpsField)
        fpsField = env->GetFieldID(clazz, "fps", "I");
    env->SetIntField(thiz, fpsField, Fps);

    static jfieldID averageFrametimeField{};
    if (!averageFrametimeField)
        averageFrametimeField = env->GetFieldID(clazz, "averageFrametime", "F");
    env->SetFloatField(thiz, averageFrametimeField, AverageFrametimeMs);

    static jfieldID averageFrametimeDeviationField{};
    if (!averageFrametimeDeviationField)
        averageFrametimeDeviationField = env->GetFieldID(clazz, "averageFrametimeDeviation", "F");
    env->SetFloatField(thiz, averageFrametimeDeviationField, AverageFrametimeDeviationMs);
}

extern "C" JNIEXPORT void JNICALL Java_emu_strato_input_InputHandler_00024Companion_setController(JNIEnv *, jobject, jint index, jint type, jint partnerIndex) {
    auto input{InputWeak.lock()};
    std::lock_guard guard(input->npad.mutex);
    input->npad.controllers[static_cast<size_t>(index)] = strato::input::GuestController{static_cast<strato::input::NpadControllerType>(type), static_cast<strato::i8>(partnerIndex)};
}

extern "C" JNIEXPORT void JNICALL Java_emu_strato_input_InputHandler_00024Companion_updateControllers(JNIEnv *, jobject) {
    InputWeak.lock()->npad.Update();
}

extern "C" JNIEXPORT void JNICALL Java_emu_strato_input_InputHandler_00024Companion_setButtonState(JNIEnv *, jobject, jint index, jlong mask, jboolean pressed) {
    auto input{InputWeak.lock()};
    if (!input)
        return; // We don't mind if we miss button updates while input hasn't been initialized
    auto device{input->npad.controllers[static_cast<size_t>(index)].device};
    if (device)
        device->SetButtonState(strato::input::NpadButton{.raw = static_cast<strato::u64>(mask)}, pressed);
}

extern "C" JNIEXPORT void JNICALL Java_emu_strato_input_InputHandler_00024Companion_setAxisValue(JNIEnv *, jobject, jint index, jint axis, jint value) {
    auto input{InputWeak.lock()};
    if (!input)
        return; // We don't mind if we miss axis updates while input hasn't been initialized
    auto device{input->npad.controllers[static_cast<size_t>(index)].device};
    if (device)
        device->SetAxisValue(static_cast<strato::input::NpadAxisId>(axis), value);
}

extern "C" JNIEXPORT void JNICALL Java_emu_strato_input_InputHandler_00024Companion_setMotionState(JNIEnv *env, jobject, jint index, jint motionId, jobject value) {
    auto input{InputWeak.lock()};
    if (!input)
        return; // We don't mind if we miss motion updates while input hasn't been initialized

    const auto motionValue = reinterpret_cast<strato::input::MotionSensorState*>(env->GetDirectBufferAddress(value));

    auto device{input->npad.controllers[static_cast<size_t>(index)].device};
    if (device)
        device->SetMotionValue(static_cast<strato::input::MotionId>(motionId), motionValue);
}

extern "C" JNIEXPORT void JNICALL Java_emu_strato_input_InputHandler_00024Companion_setTouchState(JNIEnv *env, jobject, jintArray pointsJni) {
    using Point = strato::input::TouchScreenPoint;

    auto input{InputWeak.lock()};
    if (!input)
        return; // We don't mind if we miss touch updates while input hasn't been initialized
    jboolean isCopy{false};

    strato::span<Point> points(reinterpret_cast<Point *>(env->GetIntArrayElements(pointsJni, &isCopy)),
                                static_cast<size_t>(env->GetArrayLength(pointsJni)) / (sizeof(Point) / sizeof(jint)));
    input->touch.SetState(points);
    env->ReleaseIntArrayElements(pointsJni, reinterpret_cast<jint *>(points.data()), JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL Java_emu_strato_settings_NativeSettings_updateNative(JNIEnv *env, jobject) {
    auto settings{SettingsWeak.lock()};
    if (!settings)
        return; // We don't mind if we miss settings updates while settings haven't been initialized
    settings->Update();
}
