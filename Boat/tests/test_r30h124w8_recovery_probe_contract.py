from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


class TestRecoveryProbeProductionContract(unittest.TestCase):
    def test_boat_echo_is_primary_only_and_has_no_freshness_side_effect(self):
        source = (ROOT / "Boat/source/boat_dual_radio_port.cpp").read_text()
        sequence_guard = source.index("if(!linkGuard.accept(f.sequence))return;")
        inbound_start = source.index("if(f.type==RADIO_MSG_LINK_PROBE){")
        inbound_end = source.index("bool &rxValid=", inbound_start)
        inbound = source[inbound_start:inbound_end]
        self.assertLess(sequence_guard, inbound_start)
        for term in (
            "link!=LINK_NRF_PRIMARY",
            "!BOAT_R30H121_PHYSICAL_OWNER",
            "!runtimeOwner_.worker()",
            "!sessionReady()",
            "primaryFresh(now)",
            "uint32_t(now-capturedAt)>=100U",
            "!crossSequenceAccepted",
            "RecoveryProbeKind::Challenge",
            "challenged!=f.sequence",
            "sendFrame(LINK_NRF_PRIMARY,echo)",
        ):
            self.assertIn(term, inbound)
        self.assertNotIn("noteAuthenticatedRx", inbound)
        self.assertNotIn("lastNrfRxMs_", inbound)
        self.assertNotIn("lastSessionRx_", inbound)

        header = (ROOT / "Boat/source/boat_dual_radio_port.h").read_text()
        self.assertIn("#define BOAT_R30H121_PHYSICAL_OWNER 0", header)
        self.assertIn("static_assert(BOAT_R30H121_PHYSICAL_OWNER == 0", header)
        self.assertIn("#if BOAT_R30H121_ENDPOINT_COMPILE", source)

    def test_boat_queue_accepts_only_tagged_primary_echo(self):
        source = (ROOT / "Boat/source/boat_dual_radio_port.cpp").read_text()
        start = source.index("case RADIO_MSG_LINK_PROBE:{", source.index("bool BoatDualRadioPort::queueAsyncApplication"))
        end = source.index("case RADIO_MSG_ACK:", start)
        branch = source[start:end]
        for term in (
            "if(backup||frame.flags||frame.commandId",
            "RecoveryProbeKind::Echo",
            "priority=TxPriority::Heartbeat",
        ):
            self.assertIn(term, branch)

        owner = (ROOT / "Boat/source/boat_radio_owner.h").read_text()
        start = owner.index("bool submitFeedback(")
        end = owner.index("// STOP-class reply", start)
        feedback = owner[start:end]
        for term in (
            "f.type==6U&&p==TxPriority::Heartbeat&&!f.flags&&!f.commandId",
            "RecoveryProbeKind::Echo",
            "recoveryEcho||",
        ):
            self.assertIn(term, feedback)

    def test_remote_accepts_legacy_zero_probe_and_primary_tagged_echo(self):
        source = (ROOT / "Remote/source/boat_radio.cpp").read_text()
        start = source.index("if(frame.type==RADIO_MSG_LINK_PROBE&&!payloadBytesZero")
        end = source.index("RadioSessionCounter &guard=", start)
        parser = source[start:end]
        for term in (
            "payloadBytesZero(frame.payload,0,7)",
            "link!=LINK_NRF_PRIMARY",
            "RecoveryProbeKind::Echo",
        ):
            self.assertIn(term, parser)

        sequence_guard = source.index("if(!guard.accept(frame.sequence))return;")
        dispatch_call = source.index("if(!dispatchValidatedPayload(link,frame,capturedAt))return;")
        self.assertLess(end, sequence_guard)
        self.assertLess(sequence_guard, dispatch_call)

        dispatch_start = source.index("static bool dispatchValidatedPayload(")
        dispatch_end = source.index("static bool payloadBytesZero", dispatch_start)
        dispatch = source[dispatch_start:dispatch_end]
        probe = dispatch.index("case RADIO_MSG_LINK_PROBE:")
        self.assertEqual(dispatch[probe:dispatch.index("default:", probe)].strip(),
                         "case RADIO_MSG_LINK_PROBE:\n            return true;")


if __name__ == "__main__":
    unittest.main()
