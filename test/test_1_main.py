import os
import random
import xml.etree.ElementTree as et

import numpy as np
import imageio.v3 as iio

from wfcpp import *


class TestColor:
    def test_constructing(self) -> None:
        c1 = Color()
        assert (c1.r, c1.g, c1.b) == (0, 0, 0)

        c1.r, c1.g, c1.b = 1, 2, 3
        assert (c1.r, c1.g, c1.b) == (1, 2, 3)

        c2 = Color(1, 2, 3)
        assert (c2.r, c2.g, c2.b) == (1, 2, 3)

    def test_comparing(self) -> None:
        c1 = Color()
        c1.r, c1.g, c1.b = 1, 2, 3
        c2 = Color(1, 2, 3)
        assert c1 == c2

        c3 = Color(4, 5, 6)
        assert c1 == c2 != c3


class TestColorMap:
    def test_constructing_basic(self) -> None:
        cm1 = ColorMap(2, 3)
        assert (cm1.height, cm1.width) == (2, 3)

        cm2 = ColorMap(4, 5, Color(1, 2, 3))
        assert (cm2.height, cm2.width) == (4, 5)

    def test_constructing_numpy(self) -> None:
        arr: np.ndarray = np.arange(6 * 7 * 3, dtype="uint8")
        arr = arr.reshape((6, 7, 3))
        cm1 = ColorMap(arr)
        assert (cm1.height, cm1.width) == (6, 7)

    def test_buffer_acessing(self) -> None:
        arr1: np.ndarray = np.arange(6 * 7 * 3, dtype="uint8")
        arr1 = arr1.reshape((6, 7, 3))
        cm1 = ColorMap(arr1)
        assert (cm1.height, cm1.width) == (6, 7)

        assert cm1[2, 3] == Color(arr1[2, 3, 0], arr1[2, 3, 1], arr1[2, 3, 2])
        cm1[2, 3] = Color(0, 1, 2)
        assert (cm1[2, 3].r, cm1[2, 3].g, cm1[2, 3].b) == (0, 1, 2)

        arr2 = np.array(cm1, copy=False)
        assert arr2.shape == (6, 7, 3)
        assert (arr2[2, 3] == [0, 1, 2]).all()
        assert (arr2[4, 5] == arr1[4, 5]).all()


class TestOverlappingModel:
    def test_constructing(self) -> None:
        opt1 = OverlappingModelOptions()
        # The following are built-in default values
        assert opt1.periodic_input == True
        assert opt1.periodic_output == False
        assert opt1.out_height == 48
        assert opt1.out_width == 48
        assert opt1.symmetry == 8
        assert opt1.ground == False
        assert opt1.pattern_size == 0


class OverlappingModelSample:
    def __init__(self, input: list[list[list[int]]],
                 options: OverlappingModelOptions) -> None:
        self.input = ColorMap(np.array(input, dtype="uint8"))
        self.options = options


class TestOverlappingModel:
    w = [255, 255, 255]
    b = [0, 0, 0]
    r = [255, 0, 0]

    red_maze = OverlappingModelSample(
        [[w, w, w, w],
         [w, b, b, b],
         [w, b, r, b],
         [w, b, b, b]],
        OverlappingModelOptions(
            N=2
        )
    )

    def test_constructing(self) -> None:
        model = OverlappingModel(self.red_maze.input, self.red_maze.options, 0)

    def test_running_raw(self) -> None:
        for i in range(10):
            model = OverlappingModel(self.red_maze.input,
                                     self.red_maze.options,
                                     random.randint(0, 2147483647))
            cmap = model.run()
            if cmap is not None:
                out = np.array(cmap, copy=False)
                break
        assert out is not None


class TestApp:
    def test_dedicated_io(self) -> None:
        root = et.parse("./samples/samples.xml").getroot()

        for e in root.iter("overlapping"):
            name: str = e.get("name")
            assert name is not str
            assert e.get("N") is not None
            opt = {
                "N": int(e.get("N")),
                "periodicInput": bool(e.get("periodicInput", True)),
                "periodic": bool(e.get("periodic", False)),
                "height": int(e.get("height", 48)),
                "width": int(e.get("width", 48)),
                "symmetry": int(e.get("symmetry", 8)),
                "ground": int(e.get("ground", 0))
            }
            img = iio.imread(f"./samples/{name}.png")

            for i in range(10):
                om = OverlappingModel(
                    ColorMap(img),
                    OverlappingModelOptions(**opt),
                    random.randint(0, 2147483647))
                cmap = om.run()
                if cmap is not None:
                    out = np.array(cmap, copy=False)
                    break
            assert out is not None

            # Please see the generated images in the output directory `./out`
            if not os.path.isdir("./out"):
                os.makedirs("./out")
            iio.imwrite(f"./out/{name}.png", out)
