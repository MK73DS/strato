/*
 * SPDX-License-Identifier: MPL-2.0
 * Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)
 */

package emu.strato.adapter.controller

import emu.strato.R
import emu.strato.adapter.GenericListItem
import emu.strato.adapter.GenericViewHolder
import emu.strato.databinding.ControllerItemBinding
import emu.strato.di.getInputManager
import emu.strato.input.AxisGuestEvent
import emu.strato.input.ButtonGuestEvent
import emu.strato.input.StickId

/**
 * This item is used to display all information regarding a [stick] and it's mappings for the controller
 */
class ControllerStickViewItem(private val controllerId : Int, val stick : StickId, private val onClick : (item : ControllerStickViewItem, position : Int) -> Unit) : ControllerViewItem(stick.toString()) {
    override fun bind(holder : GenericViewHolder<ControllerItemBinding>, position : Int) {
        val binding = holder.binding
        val context = binding.root.context
        val inputManager = context.getInputManager()

        val buttonGuestEvent = ButtonGuestEvent(controllerId, stick.button)
        val button = inputManager.eventMap.filter { it.value is ButtonGuestEvent && it.value == buttonGuestEvent }.keys.firstOrNull()?.toString() ?: context.getString(R.string.none)

        var axisGuestEvent = AxisGuestEvent(controllerId, stick.yAxis, true)
        val yAxisPlus = inputManager.eventMap.filter { it.value is AxisGuestEvent && it.value == axisGuestEvent }.keys.firstOrNull()?.toString() ?: context.getString(R.string.none)

        axisGuestEvent = AxisGuestEvent(controllerId, stick.yAxis, false)
        val yAxisMinus = inputManager.eventMap.filter { it.value is AxisGuestEvent && it.value == axisGuestEvent }.keys.firstOrNull()?.toString() ?: context.getString(R.string.none)

        axisGuestEvent = AxisGuestEvent(controllerId, stick.xAxis, true)
        val xAxisPlus = inputManager.eventMap.filter { it.value is AxisGuestEvent && it.value == axisGuestEvent }.keys.firstOrNull()?.toString() ?: context.getString(R.string.none)

        axisGuestEvent = AxisGuestEvent(controllerId, stick.xAxis, false)
        val xAxisMinus = inputManager.eventMap.filter { it.value is AxisGuestEvent && it.value == axisGuestEvent }.keys.firstOrNull()?.toString() ?: context.getString(R.string.none)

        subContent = "${context.getString(R.string.button)}: $button\n${context.getString(R.string.up)}: $yAxisPlus\n${context.getString(R.string.down)}: $yAxisMinus\n${context.getString(R.string.left)}: $xAxisMinus\n${context.getString(R.string.right)}: $xAxisPlus"

        super.bind(holder, position)

        binding.root.setOnClickListener { onClick.invoke(this, position) }
    }

    override fun areItemsTheSame(other : GenericListItem<ControllerItemBinding>) = other is ControllerStickViewItem && controllerId == other.controllerId

    override fun areContentsTheSame(other : GenericListItem<ControllerItemBinding>) = other is ControllerStickViewItem && stick == other.stick
}
