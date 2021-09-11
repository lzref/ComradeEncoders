import Live
import _Framework.Capabilities as caps

from .comrade_midi import ComradeMidi

def get_capabilities():
    return {caps.CONTROLLER_ID_KEY: caps.controller_id(vendor_id=1452, product_ids=[
            33417], model_name='Maple'),
            caps.PORTS_KEY: [
                caps.inport(props=[]),
                caps.inport(props=[caps.NOTES_CC, caps.SCRIPT, caps.REMOTE]),
                caps.outport(props=[]),
                caps.outport(props=[caps.NOTES_CC, caps.SCRIPT, caps.REMOTE])]}



def create_instance(c_instance):
    return ComradeMidi(c_instance=c_instance)
