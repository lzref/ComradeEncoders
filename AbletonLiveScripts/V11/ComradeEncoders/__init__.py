from __future__ import absolute_import, print_function, unicode_literals
from ableton.v2.control_surface.capabilities import CONTROLLER_ID_KEY, NOTES_CC, PORTS_KEY, REMOTE, SCRIPT, controller_id, inport, outport
from .comrade_encoders import ComradeEncoders

def get_capabilities():
    return {CONTROLLER_ID_KEY: controller_id(vendor_id=1452, product_ids=[33417], model_name=[u'Maple']),
     PORTS_KEY: [inport(props=[NOTES_CC, REMOTE]),
                 inport(props=[NOTES_CC, SCRIPT, REMOTE]),
                 inport(props=[NOTES_CC, REMOTE]),
                 outport(props=[]),
                 outport(props=[SCRIPT]),
                 outport(props=[]),
                 outport(props=[])]}


def create_instance(c_instance):
    return ComradeEncoders(c_instance=c_instance)
