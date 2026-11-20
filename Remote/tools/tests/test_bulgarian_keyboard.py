import pathlib
import re
import unittest

class BulgarianKeyboardTests(unittest.TestCase):
    def test_phonetic_layout(self):
        path = pathlib.Path(__file__).resolve().parents[2] / "source" / "bulgarian_keyboard.h"
        source = path.read_text(encoding="utf-8")
        expected = "яв ертъуиопшщ асдфгхйклч зьцжбнмю".replace(" ", "")
        lower = re.search(r'bgLower\[\]\s*=\s*\{(.*?)\};', source, re.S).group(1)
        upper = re.search(r'bgUpper\[\]\s*=\s*\{(.*?)\};', source, re.S).group(1)
        letters = "".join(re.findall(r'"([а-я])"', lower))
        capitals = "".join(re.findall(r'"([А-Я])"', upper))
        self.assertEqual(letters, expected)
        self.assertEqual(capitals, expected.upper())
        self.assertEqual(set(letters), set("абвгдежзийклмнопрстуфхцчшщъьюя"))
        self.assertEqual(len(letters), 30)
        self.assertIn("bgCtrl[35]", source)

if __name__ == "__main__":
    unittest.main()
