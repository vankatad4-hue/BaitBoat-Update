from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


class TestRemoteCooperativeLoopContract(unittest.TestCase):
    def test_centre_calibration_starts_without_sampling_loop(self):
        source = (ROOT / "Remote/source/joystick.cpp").read_text()
        start = source.index("bool joystick_calibrateCentre()")
        end = source.index("static void serviceCentre", start)
        begin = source[start:end]
        self.assertIn("centreSampler.start(millis())", begin)
        self.assertIn("centreState = CENTRE_RUNNING", begin)
        for blocking in ("delay(", "while(", "for("):
            self.assertNotIn(blocking, begin)

    def test_radio_joystick_and_display_keep_one_loop_owner(self):
        source = (ROOT / "Remote/source/Flytec_Remote.ino").read_text()
        loop = source[source.index("void loop()") :]
        radio = loop.index("boatRadio_update();")
        joystick = loop.index("joystick_update();")
        display = loop.index("displayPort_loop();")
        self.assertLess(radio, joystick)
        self.assertLess(joystick, display)

    def test_map_reader_is_cooperative_and_bounded_per_renderer_update(self):
        header = (ROOT / "Remote/source/offline_map.h").read_text()
        source = (ROOT / "Remote/source/offline_map.cpp").read_text()
        renderer = (ROOT / "Remote/source/map_renderer.cpp").read_text()
        self.assertNotIn("offlineMap_readTile", header + source + renderer)
        self.assertIn("constexpr size_t chunk=4096", source)
        self.assertIn("offlineMap_stepTile(buf)", renderer)
        self.assertIn("if(loadsThisUpdate>=1||pendingTile)continue", renderer)
        self.assertIn("if(pendingTile){stepTile();loadsThisUpdate=1;}", renderer)


if __name__ == "__main__":
    unittest.main()
