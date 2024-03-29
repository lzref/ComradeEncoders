from __future__ import absolute_import, print_function, unicode_literals
from contextlib import contextmanager
from functools import partial
import Live
from ableton.v2.base import const, inject, listens
from ableton.v2.control_surface import BankingInfo, DeviceBankRegistry, DeviceDecoratorFactory, IdentifiableControlSurface, Layer, MIDI_CC_TYPE, PercussionInstrumentFinder
from ableton.v2.control_surface.components import AutoArmComponent, BackgroundComponent, ClipActionsComponent, DrumGroupComponent, RightAlignTracksTrackAssigner
from ableton.v2.control_surface.default_bank_definitions import BANK_DEFINITIONS
from ableton.v2.control_surface.mode import AddLayerMode, LayerMode, ModesComponent, NullModes, ReenterBehaviour, SetAttributeMode
from novation.colors import CLIP_COLOR_TABLE, RGB_COLOR_TABLE, Rgb
from . import sysex
from .actions import ActionsComponent
from .device import DeviceComponent
from .device_navigation import DisplayingDeviceNavigationComponent, NUM_VISIBLE_ITEMS
from .device_parameters import DeviceParameterComponent
from .elements import Elements
from .message import MessageComponent
from .midi_message_cache import MidiMessageCache
from .mode import DisplayingNavigatableModesComponent, DisplayingSkinableModesComponent
from .skin import skin
from .transport import TransportComponent
from .util import is_song_recording
DRUM_FEEDBACK_CHANNEL = 4

class ComradeEncoders(IdentifiableControlSurface):
    _sysex_message_cache = MidiMessageCache()

    def __init__(self, *a, **k):
        super(ComradeEncoders, self).__init__((), *a, **k)
        self._main_modes = NullModes()
        self._element_injector = inject(element_container=const(None)).everywhere()
        self._message_injector = inject(message=const(None)).everywhere()
        with self.component_guard():
            with inject(skin=const(skin), message_cache=const(self._sysex_message_cache)).everywhere():
                self._elements = Elements()
        self._element_injector = inject(element_container=const(self._elements)).everywhere()
        with self.component_guard():
            self._create_message()
        self._message_injector = inject(message=const(self._message)).everywhere()
        self._switch_display_layout(sysex.KNOB_SCREEN_LAYOUT_BYTE)
        self._device_bank_registry = DeviceBankRegistry()
        with self.component_guard():
            self._create_transport()
            self._create_auto_arm()
            self._create_drums()
            self._create_device()
            self._create_device_navigation()
            self._create_actions()
            self._create_clip_actions()
            self._create_background()
            self._create_modes()
        self._drum_group_finder = self.register_disconnectable(PercussionInstrumentFinder(device_parent=self.song.view.selected_track))
        self.__on_drum_group_found.subject = self._drum_group_finder
        self.__on_drum_group_found()
        self.__on_selected_track_changed.subject = self.song.view
        self.__on_selected_track_changed()
        self.__on_record_mode_changed.subject = self.song
        self.set_feedback_channels([DRUM_FEEDBACK_CHANNEL])
        self._set_feedback_velocity()

    def on_identified(self, midi_bytes):
        self._switch_display_layout(sysex.KNOB_SCREEN_LAYOUT_BYTE, force=True)
        self._main_modes.selected_mode = u'device_control'
        self._auto_arm.set_enabled(True)
        self.set_feedback_channels([DRUM_FEEDBACK_CHANNEL])
        super(ComradeEncoders, self).on_identified(midi_bytes)

    def disconnect(self):
        self._auto_arm.set_enabled(False)
        super(ComradeEncoders, self).disconnect()

    def port_settings_changed(self):
        self._auto_arm.set_enabled(False)
        super(ComradeEncoders, self).port_settings_changed()

    @contextmanager
    def _component_guard(self):
        with super(ComradeEncoders, self)._component_guard():
            with self._element_injector:
                with self._message_injector:
                    yield
        self._format_and_send_sysex()

    def _format_and_send_sysex(self):
        messages_to_send = sysex.make_sysex_from_segments(self._sysex_message_cache.messages)
        for msg in messages_to_send:
            self._send_midi(msg)

        self._sysex_message_cache.clear()

    def _install_mapping(self, midi_map_handle, control, parameter, feedback_delay, feedback_map):
        success = False
        if control.message_type() == MIDI_CC_TYPE:
            feedback_rule = Live.MidiMap.CCFeedbackRule()
            feedback_rule.cc_no = control.message_identifier()
            feedback_rule.cc_value_map = feedback_map
            feedback_rule.channel = getattr(control, u'feedback_channel', control.message_channel())
            feedback_rule.delay_in_ms = feedback_delay
            success = Live.MidiMap.map_midi_cc_with_feedback_map(midi_map_handle, parameter, control.message_channel(), control.message_identifier(), control.message_map_mode(), feedback_rule, not control.needs_takeover(), control.mapping_sensitivity)
            if success:
                Live.MidiMap.send_feedback_for_parameter(midi_map_handle, parameter)
        return success

    def _create_message(self):
        self._message = MessageComponent(name=u'Message', is_enabled=False, layer=Layer(display=u'message_display'))
        self._message.set_enabled(True)

    def _switch_display_layout(self, layout_byte, force = False):
        display_layout_switch = self._elements.display_layout_switch
        if force:
            display_layout_switch.clear_send_cache()
        display_layout_switch.send_value(layout_byte)
        self._clear_display_send_cache()

    def _clear_display_send_cache(self):
        for display in self._elements.text_display_lines:
            display.clear_send_cache()

    def _create_transport(self):
        self._transport = TransportComponent(name=u'Transport', is_enabled=False, layer=Layer(play_button=u'play_button', stop_button=u'stop_button', seek_backward_button=u'rw_button', seek_forward_button=u'ff_button', loop_button=u'loop_button', record_button=u'record_button', continue_playing_button=u'play_button_with_shift'))
        self._transport.set_enabled(True)

    def _create_auto_arm(self):
        self._auto_arm = AutoArmComponent(is_enabled=False, name=u'Auto_Arm')

    def _create_drums(self):
        self._drum_group = DrumGroupComponent(name=u'Drum_Group', translation_channel=DRUM_FEEDBACK_CHANNEL)

    def _create_device(self):
        self._banking_info = BankingInfo(BANK_DEFINITIONS)
        self._device = DeviceComponent(device_decorator_factory=DeviceDecoratorFactory(), device_bank_registry=self._device_bank_registry, banking_info=self._banking_info, name=u'Device')
        self._device_parameters = DeviceParameterComponent(parameter_provider=self._device, name=u'Device_Parameters')

    def _create_device_navigation(self):
        self._device_navigation = DisplayingDeviceNavigationComponent(banking_info=self._banking_info, device_bank_registry=self._device_bank_registry, device_component=self._device, num_visible_items=NUM_VISIBLE_ITEMS, name=u'Device_Navigation')

    def _create_actions(self):
        self._actions = ActionsComponent(name=u'Actions', is_enabled=False, layer=Layer(actions_color_fields=u'color_field_line_2_with_shift', undo_button=u'select_buttons_with_shift_raw[0]', redo_button=u'select_buttons_with_shift_raw[1]', metronome_button=u'select_buttons_with_shift_raw[2]', capture_midi_button=u'select_buttons_with_shift_raw[7]'))
        self._actions.set_enabled(True)

    def _create_clip_actions(self):
        self._clip_actions = ClipActionsComponent(name=u'Clip_Actions', is_enabled=False, layer=Layer(delete_button=u'clear_button', duplicate_button=u'duplicate_button', double_loop_button=u'duplicate_button_with_shift'))
        self._clip_actions.set_enabled(True)

    def _create_background(self):
        self._background = BackgroundComponent(name=u'Background', is_enabled=False, add_nop_listeners=True, layer=Layer(select_button_7_with_shift=u'select_buttons_with_shift_raw[3]', select_button_4_with_shift=u'select_buttons_with_shift_raw[4]', select_button_5_with_shift=u'select_buttons_with_shift_raw[5]', select_button_6_with_shift=u'select_buttons_with_shift_raw[6]'))
        self._background.set_enabled(True)

    def _create_modes(self):
        self._encoder_modes = DisplayingSkinableModesComponent(name=u'Encoder_Modes')
        self._encoder_modes.add_mode(u'devices', [partial(self._switch_display_layout, sysex.BOX_SCREEN_LAYOUT_BYTE),
         AddLayerMode(self._encoder_modes, Layer(mode_display=self._elements.text_display_line_5, mode_color_fields=self._elements.color_field_line_2, mode_selection_fields=self._elements.selection_field_line_2)),
         LayerMode(self._device_navigation, Layer(select_buttons=u'pads_flattened', device_color_fields=u'color_field_lines_0_1_flattened', device_name_display_1=u'text_display_line_0', device_name_display_2=u'text_display_line_2', device_bank_name_display_1=u'text_display_line_1', device_bank_name_display_2=u'text_display_line_3', device_selection_fields=u'selection_field_lines_0_1_flattened', selected_device_name_display=u'center_display_1')),
         SetAttributeMode(self._device_navigation, u'scroll_left_layer', Layer(button=u'up_button')),
         SetAttributeMode(self._device_navigation, u'scroll_right_layer', Layer(button=u'down_button')),
         LayerMode(self._device, Layer(prev_bank_button=u'display_up_button', next_bank_button=u'display_down_button')),
         AddLayerMode(self._background, Layer(center_display_2=u'center_display_2', scene_launch_buttons=u'scene_launch_buttons', encoders=u'encoders')),
         AddLayerMode(self._actions, Layer(actions_display=u'text_display_line_5_with_shift', actions_selection_fields=u'selection_field_line_2_with_shift'))])
        self._encoder_modes.add_mode(u'pan', [partial(self._switch_display_layout, sysex.KNOB_SCREEN_LAYOUT_BYTE),
         AddLayerMode(self._encoder_modes, Layer(mode_display=self._elements.text_display_line_3, mode_color_fields=self._elements.color_field_line_2, mode_selection_fields=self._elements.selection_field_line_1, selected_mode_color_field=u'center_color_field')),
         AddLayerMode(self._background, Layer(display_up_button=u'display_up_button', display_down_button=u'display_down_button')),
         AddLayerMode(self._actions, Layer(actions_display=u'text_display_line_3_with_shift', actions_selection_fields=u'selection_field_line_1_with_shift'))])
        self._encoder_modes.add_mode(u'sends', [partial(self._switch_display_layout, sysex.KNOB_SCREEN_LAYOUT_BYTE),
         AddLayerMode(self._encoder_modes, Layer(mode_display=self._elements.text_display_line_3, mode_color_fields=self._elements.color_field_line_2, mode_selection_fields=self._elements.selection_field_line_1, selected_mode_color_field=u'center_color_field')),
         AddLayerMode(self._actions, Layer(actions_display=u'text_display_line_3_with_shift', actions_selection_fields=u'selection_field_line_1_with_shift'))])
        self._pad_modes = ModesComponent(name=u'Pad_Modes')
        self._pad_modes.add_mode(u'drum', LayerMode(self._drum_group, Layer(matrix=u'pads_quadratic', scroll_up_button=u'up_button', scroll_down_button=u'down_button')))
        self._pad_modes.add_mode(u'disabled', AddLayerMode(self._background, Layer(matrix=u'pads_quadratic', scroll_up_button=u'up_button', scroll_down_button=u'down_button')))
        self._main_modes = ModesComponent(name=u'Encoder_Modes')
        set_main_mode = partial(setattr, self._main_modes, u'selected_mode')
        self._main_modes.add_mode(u'device_control', [partial(self._switch_display_layout, sysex.KNOB_SCREEN_LAYOUT_BYTE),
         LayerMode(self._device_parameters, Layer(parameter_controls=u'encoders', name_display_line=u'text_display_line_0', value_display_line=u'text_display_line_1', parameter_color_fields=u'color_field_line_0', encoder_color_fields=u'encoder_color_fields')),
         LayerMode(self._device, Layer(prev_bank_button=u'display_up_button', next_bank_button=u'display_down_button')),
         LayerMode(self._device_navigation, Layer(selected_device_name_display=u'center_display_1', selected_device_bank_name_display=u'center_display_2')),
         AddLayerMode(self._actions, Layer(actions_display=u'text_display_line_3_with_shift', actions_selection_fields=u'selection_field_line_1_with_shift'))])
        self._main_modes.add_mode(u'options', [self._encoder_modes,
         LayerMode(self._encoder_modes, Layer(devices_button=u'select_buttons_raw[0]', pan_button=u'select_buttons_raw[1]', sends_button=u'select_buttons_raw[2]')),
         SetAttributeMode(self._encoder_modes, u'selected_mode', u'devices'),
         AddLayerMode(self._background, Layer(select_button_3=u'select_buttons_raw[3]', select_button_4=u'select_buttons_raw[4]', select_button_5=u'select_buttons_raw[5]', select_button_6=u'select_buttons_raw[6]', select_button_7=u'select_buttons_raw[7]'))], behaviour=ReenterBehaviour(on_reenter=partial(set_main_mode, u'device_control')))
        self._main_modes.add_mode(u'grid', [partial(self._switch_display_layout, sysex.KNOB_SCREEN_LAYOUT_BYTE),
         self._pad_modes,
         self._select_grid_mode,
         LayerMode(self._device_parameters, Layer(parameter_controls=u'encoders', name_display_line=u'text_display_line_0', value_display_line=u'text_display_line_1', parameter_color_fields=u'color_field_line_0', encoder_color_fields=u'encoder_color_fields')),
         LayerMode(self._device, Layer(prev_bank_button=u'display_up_button', next_bank_button=u'display_down_button')),
         LayerMode(self._device_navigation, Layer(selected_device_name_display=u'center_display_1', selected_device_bank_name_display=u'center_display_2')),
         AddLayerMode(self._background, Layer(scene_launch_buttons=u'scene_launch_buttons')),
         AddLayerMode(self._actions, Layer(actions_display=u'text_display_line_3_with_shift', actions_selection_fields=u'selection_field_line_1_with_shift'))], behaviour=ReenterBehaviour(on_reenter=partial(set_main_mode, u'device_control')))
        self._main_modes.layer = Layer(options_button=u'options_button', grid_button=u'grid_button')
        self._main_modes.selected_mode = u'device_control'

    @listens(u'instrument')
    def __on_drum_group_found(self):
        self._drum_group.set_drum_group_device(self._drum_group_finder.drum_group)
        self._select_grid_mode()

    @listens(u'selected_track')
    def __on_selected_track_changed(self):
        track = self.song.view.selected_track
        self.__on_selected_track_implicit_arm_changed.subject = track
        self._drum_group_finder.device_parent = track
        self._select_grid_mode()

    @listens(u'implicit_arm')
    def __on_selected_track_implicit_arm_changed(self):
        self._set_feedback_velocity()

    @listens(u'record_mode')
    def __on_record_mode_changed(self):
        self._set_feedback_velocity()

    def _select_grid_mode(self):
        if self._main_modes.selected_mode == u'grid':
            drum_device = self._drum_group_finder.drum_group
            self._pad_modes.selected_mode = u'drum' if drum_device else u'disabled'
            if drum_device:
                self.set_controlled_track(self.song.view.selected_track)
                self._set_feedback_velocity()
            else:
                self.release_controlled_track()

    def _set_feedback_velocity(self):
        if is_song_recording(self.song) and self.song.view.selected_track.implicit_arm:
            feedback_velocity = Rgb.RED.midi_value
        else:
            feedback_velocity = Rgb.GREEN.midi_value
        self._c_instance.set_feedback_velocity(int(feedback_velocity))
