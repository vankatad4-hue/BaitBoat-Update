import unittest
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from mbtiles_area_export import mbtiles_tms_to_xyz_y, rgb565_le
from PIL import Image

class MBTilesAreaExportTests(unittest.TestCase):
    def test_tms_to_xyz_flip(self):
        self.assertEqual(mbtiles_tms_to_xyz_y(0,3),7)
        self.assertEqual(mbtiles_tms_to_xyz_y(7,3),0)
    def test_rgb565_tile_size_and_red_pixel(self):
        image=Image.new("RGB",(256,256),(255,0,0))
        data=rgb565_le(image)
        self.assertEqual(len(data),131072)
        self.assertEqual(data[:2],bytes((0x00,0xF8)))

if __name__=="__main__": unittest.main()
