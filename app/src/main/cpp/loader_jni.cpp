// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "strato/crypto/key_store.h"
#include "strato/vfs/nca.h"
#include "strato/vfs/os_backing.h"
#include "strato/vfs/os_filesystem.h"
#include "strato/loader/nro.h"
#include "strato/loader/nso.h"
#include "strato/loader/nca.h"
#include "strato/loader/xci.h"
#include "strato/loader/nsp.h"
#include "strato/jvm.h"

extern "C" JNIEXPORT jint JNICALL Java_emu_strato_loader_RomFile_populate(JNIEnv *env, jobject thiz, jint jformat, jint fd, jstring appFilesPathJstring, jint systemLanguage) {
    strato::signal::ScopedStackBlocker stackBlocker;

    strato::loader::RomFormat format{static_cast<strato::loader::RomFormat>(jformat)};

    auto keyStore{std::make_shared<strato::crypto::KeyStore>(strato::JniString(env, appFilesPathJstring))};
    std::unique_ptr<strato::loader::Loader> loader;
    try {
        auto backing{std::make_shared<strato::vfs::OsBacking>(fd)};

        switch (format) {
            case strato::loader::RomFormat::NRO:
                loader = std::make_unique<strato::loader::NroLoader>(backing);
                break;
            case strato::loader::RomFormat::NSO:
                loader = std::make_unique<strato::loader::NsoLoader>(backing);
                break;
            case strato::loader::RomFormat::NCA:
                loader = std::make_unique<strato::loader::NcaLoader>(backing, keyStore);
                break;
            case strato::loader::RomFormat::XCI:
                loader = std::make_unique<strato::loader::XciLoader>(backing, keyStore);
                break;
            case strato::loader::RomFormat::NSP:
                loader = std::make_unique<strato::loader::NspLoader>(backing, keyStore);
                break;
            default:
                return static_cast<jint>(strato::loader::LoaderResult::ParsingError);
        }
    } catch (const strato::loader::loader_exception &e) {
        return static_cast<jint>(e.error);
    } catch (const std::exception &e) {
        return static_cast<jint>(strato::loader::LoaderResult::ParsingError);
    }

    jclass clazz{env->GetObjectClass(thiz)};
    jfieldID applicationNameField{env->GetFieldID(clazz, "applicationName", "Ljava/lang/String;")};
    jfieldID applicationTitleIdField{env->GetFieldID(clazz, "applicationTitleId", "Ljava/lang/String;")};
    jfieldID applicationAuthorField{env->GetFieldID(clazz, "applicationAuthor", "Ljava/lang/String;")};
    jfieldID rawIconField{env->GetFieldID(clazz, "rawIcon", "[B")};
    jfieldID applicationVersionField{env->GetFieldID(clazz, "applicationVersion", "Ljava/lang/String;")};

    if (loader->nacp) {
        auto language{strato::language::GetApplicationLanguage(static_cast<strato::language::SystemLanguage>(systemLanguage))};
        if (((1 << static_cast<strato::u32>(language)) & loader->nacp->supportedTitleLanguages) == 0)
            language = loader->nacp->GetFirstSupportedTitleLanguage();

        env->SetObjectField(thiz, applicationNameField, env->NewStringUTF(loader->nacp->GetApplicationName(language).c_str()));
        env->SetObjectField(thiz, applicationVersionField, env->NewStringUTF(loader->nacp->GetApplicationVersion().c_str()));
        env->SetObjectField(thiz, applicationTitleIdField, env->NewStringUTF(loader->nacp->GetSaveDataOwnerId().c_str()));
        env->SetObjectField(thiz, applicationAuthorField, env->NewStringUTF(loader->nacp->GetApplicationPublisher(language).c_str()));

        auto icon{loader->GetIcon(language)};
        jbyteArray iconByteArray{env->NewByteArray(static_cast<jsize>(icon.size()))};
        env->SetByteArrayRegion(iconByteArray, 0, static_cast<jsize>(icon.size()), reinterpret_cast<const jbyte *>(icon.data()));
        env->SetObjectField(thiz, rawIconField, iconByteArray);
    }

    return static_cast<jint>(strato::loader::LoaderResult::Success);
}

extern "C" JNIEXPORT jstring Java_emu_strato_preference_FirmwareImportPreference_fetchFirmwareVersion(JNIEnv *env, jobject thiz, jstring systemArchivesPathJstring, jstring keysPathJstring) {
    struct SystemVersion {
        strato::u8 major;
        strato::u8 minor;
        strato::u8 micro;
        strato::u8 _pad0_;
        strato::u8 revisionMajor;
        strato::u8 revisionMinor;
        strato::u8 _pad1_[2];
        strato::u8 platformString[0x20];
        strato::u8 versionHash[0x40];
        strato::u8 displayVersion[0x18];
        strato::u8 displayTitle[0x80];
    };

    constexpr strato::u64 systemVersionProgramId{0x0100000000000809};

    auto systemArchivesFileSystem{std::make_shared<strato::vfs::OsFileSystem>(strato::JniString(env, systemArchivesPathJstring))};
    auto systemArchives{systemArchivesFileSystem->OpenDirectory("")};
    auto keyStore{std::make_shared<strato::crypto::KeyStore>(strato::JniString(env, keysPathJstring))};

    for (const auto &entry : systemArchives->Read()) {
        std::shared_ptr<strato::vfs::Backing> backing{systemArchivesFileSystem->OpenFile(entry.name)};
        auto nca{strato::vfs::NCA(backing, keyStore)};

        if (nca.header.programId == systemVersionProgramId && nca.romFs != nullptr) {
            auto controlRomFs{std::make_shared<strato::vfs::RomFileSystem>(nca.romFs)};
            auto file{controlRomFs->OpenFile("file")};
            SystemVersion systemVersion;
            file->Read<SystemVersion>(systemVersion);
            return env->NewStringUTF(reinterpret_cast<char *>(systemVersion.displayVersion));
        }
    }

    return env->NewStringUTF("");
}

std::vector<strato::u8> decodeBfttfFont(const std::shared_ptr<strato::vfs::Backing> bfttfFile){
    constexpr strato::u32 fontKey{0x06186249};
    constexpr strato::u32 BFTTFMagic{0x18029a7f};

    auto firstBytes{bfttfFile->Read<strato::u32>()};
    auto firstBytesXor{firstBytes ^ fontKey};

    if (firstBytesXor == BFTTFMagic) {
        constexpr size_t initialOffset{8};
        std::vector<strato::u8> font(bfttfFile->size - initialOffset);

        for (size_t offset = initialOffset; offset < bfttfFile->size; offset += 4) {
            strato::u32 decodedData{bfttfFile->Read<strato::u32>(offset) ^ fontKey};

            font[offset - 8] = static_cast<strato::u8>(decodedData >> 0);
            font[offset - 7] = static_cast<strato::u8>(decodedData >> 8);
            font[offset - 6] = static_cast<strato::u8>(decodedData >> 16);
            font[offset - 5] = static_cast<strato::u8>(decodedData >> 24);
        }

        return font;
    }
    return {};
}

extern "C" JNIEXPORT void Java_emu_strato_preference_FirmwareImportPreference_extractFonts(JNIEnv *env, jobject thiz, jstring systemArchivesPathJstring, jstring keysPathJstring, jstring fontsPath) {
    // Fonts are stored in the following NCAs
    // 0x0100000000000810 -> "FontNintendoExtended"
    // 0x0100000000000811 -> "FontStandard"
    // 0x0100000000000812 -> "FontKorean"
    // 0x0100000000000813 -> "FontChineseTraditional"
    // 0x0100000000000814 -> "FontChineseSimplified"

    constexpr strato::u64 firstFontProgramId{0x0100000000000810};
    constexpr strato::u64 lastFontProgramId{0x0100000000000814};

    const std::map<std::string, std::string> sharedFontFilenameDictionary = {
        {"nintendo_ext_003.bfttf", "FontNintendoExtended"},
        {"nintendo_ext2_003.bfttf", "FontNintendoExtended2"},
        {"nintendo_udsg-r_std_003.bfttf", "FontStandard"},
        {"nintendo_udsg-r_ko_003.bfttf", "FontKorean"},
        {"nintendo_udjxh-db_zh-tw_003.bfttf", "FontChineseTraditional"},
        {"nintendo_udsg-r_org_zh-cn_003.bfttf", "FontChineseSimplified"},
        {"nintendo_udsg-r_ext_zh-cn_003.bfttf", "FontExtendedChineseSimplified"}
    };

    auto fontsFileSystem{std::make_shared<strato::vfs::OsFileSystem>(strato::JniString(env, fontsPath))};
    auto systemArchivesFileSystem{std::make_shared<strato::vfs::OsFileSystem>(strato::JniString(env, systemArchivesPathJstring))};
    auto systemArchives{systemArchivesFileSystem->OpenDirectory("")};
    auto keyStore{std::make_shared<strato::crypto::KeyStore>(strato::JniString(env, keysPathJstring))};

    for (const auto &entry : systemArchives->Read()) {
        std::shared_ptr<strato::vfs::Backing> backing{systemArchivesFileSystem->OpenFile(entry.name)};
        auto nca{strato::vfs::NCA(backing, keyStore)};

        if (nca.header.programId >= firstFontProgramId && nca.header.programId <= lastFontProgramId && nca.romFs != nullptr) {
            auto controlRomFs{std::make_shared<strato::vfs::RomFileSystem>(nca.romFs)};

            for (auto fileEntry = controlRomFs->fileMap.begin(); fileEntry != controlRomFs->fileMap.end(); fileEntry++) {
                auto fileName{fileEntry->first};
                auto bfttfFile{controlRomFs->OpenFile(fileName)};

                auto decodedFont{decodeBfttfFont(bfttfFile)};
                if (decodedFont.empty())
                    continue;

                auto ttfFileName{sharedFontFilenameDictionary.at(fileName) + ".ttf"};
                if (fontsFileSystem->FileExists(ttfFileName))
                    fontsFileSystem->DeleteFile(ttfFileName);

                fontsFileSystem->CreateFile(ttfFileName, decodedFont.size());
                std::shared_ptr<strato::vfs::Backing> ttfFile{fontsFileSystem->OpenFile(ttfFileName, {true, true, false})};

                ttfFile->Write(decodedFont);
            }
        }
    }
}
