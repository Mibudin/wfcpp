import os
import random
import argparse
import cmd
import xml.etree.ElementTree as et
import time
from dataclasses import dataclass
# from typing import Any

import colorama as clr
import numpy as np
import imageio.v3 as iio

from wfcpp import *


@dataclass
class OverlappingModelSample:
    name: str
    options: OverlappingModelOptions
    input: ColorMap
    seed: int
    output: ColorMap | None = None
    iter: int = 0
    total_time: float = 0.0
    proccess_time: float = 0.0


class WFCppSampleConfig:
    def __init__(self, path: str) -> None:
        self.path: str = path
        self.root: et.Element = None
        self.overlappings: list[OverlappingModelSample] = []
        self.simpletileds: list = []

    def parse(self) -> bool:
        self.root = et.parse(self.path).getroot()
        return self._check_format()

    def _check_format(self) -> bool:
        if self.root.tag != "samples":
            return False
        return True

    def load(self, models: list[str] = ["overlapping"],
             input: str = None, names: list[str] | None = None) -> None:
        if input is None or models is None:
            return

        if "overlapping" in models:
            for e in self.root.findall("overlapping"):
                name = e.get("name")
                if (names is not None) and (name not in names):
                    continue
                sample = self._gen_overlapping_sample(
                    e, name, f"{input}/{name}.png")
                if sample is None:
                    # TODO: check error
                    continue
                self.overlappings.append(sample)

        if "simpletiled" in models:
            pass

    @classmethod
    def _gen_overlapping_sample(cls, e: et.Element, name: str, img_path: str) \
            -> OverlappingModelSample | None:
        options = cls._parse_overlapping_options(e)
        if options is None:
            # TODO: check error
            return None
        input = cls._load_overlapping_input(img_path)
        if input is None:
            # TODO: check error
            return None

        return OverlappingModelSample(
            name=name,
            options=options,
            input=input,
            seed=random.randint(0, 2147483647)
        )

    @classmethod
    def _parse_overlapping_options(cls, e: et.Element) \
            -> OverlappingModelOptions | None:
        if set(e.attrib.keys()) < set([
                "N",
                "periodicInput",
                "periodic",
                "height",
                "width",
                "symmetry",
                "ground"]):
            return None

        return OverlappingModelOptions(
            N=int(e.get("N")),
            periodicInput=bool(e.get("periodicInput", True)),
            periodic=bool(e.get("periodic", False)),
            height=int(e.get("height", 48)),
            width=int(e.get("width", 48)),
            symmetry=int(e.get("symmetry", 8)),
            ground=int(e.get("ground", 0))
        )

    @classmethod
    def _load_overlapping_input(cls, path: str) -> ColorMap | None:
        if not os.path.isfile(path):
            return None
        img = iio.imread(path)
        if img.dtype != "uint8":
            img = img.astype("uint8")
        return ColorMap(img)

    def save_out(self, output_dir: str) -> None:
        for sample in self.overlappings:
            if sample.output is None:
                continue
            iio.imwrite(f"{output_dir}/{sample.name}.png", sample.output)


@dataclass
class WFCppCLIArgs:
    config: str
    input: str
    name: list[str] | None
    trial: int = 1
    output: str = "out"
    verbose: bool = False
    color: bool = False


class WFCppCLI:
    def __init__(self) -> None:
        self.args: WFCppCLIArgs = None
        self.config: WFCppSampleConfig = None

        self._parse_args()

    def _parse_args(self) -> None:
        parser = argparse.ArgumentParser(
            prog="wfcpp-cli",
            description="WFCpp - CLI Application",
            epilog="Thank you for using WFCpp! " +
                "Please go to https://github.com/Mibudin/wfcpp for more " +
                "information!"
        )

        parser.add_argument("config", type=str,
                            help="the path to the XML configuration file")
        parser.add_argument("input", type=str,
                            help="the path to the directory of inputs")
        parser.add_argument("-n", "--name", type=str, nargs="+",
                            help="the names of targets to be proceeded")
        parser.add_argument("-t", "--trial", type=int, default=1,
                            help="the maximum number of WFC trials on a " +
                                 "target")
        parser.add_argument("-o", "--output", type=str, default="out",
                            help="the path to the output directory")
        parser.add_argument("-v", "--verbose", action="store_true",
                            help="increase the verbosity of the output " +
                                 "messages")
        parser.add_argument("-c", "--color", action="store_true",
                            help="colorize the output messages")

        args = parser.parse_args()
        self.args = WFCppCLIArgs(**vars(args))

        self._check_paths()
        if self.args.color:
            self._colorize()

    def _check_paths(self) -> None:
        if not os.path.isfile(self.args.config):
            raise argparse.ArgumentError(
                None, "the XML configuration file does not exist!")
        if not os.path.isdir(self.args.input):
            raise argparse.ArgumentError(
                None, "the directory of inputs does not exist!")
        if not os.path.isdir(self.args.output):
            os.makedirs(self.args.output)

    def _colorize(self) -> None:
        clr.init(autoreset=True)

    def _decolorize(self) -> None:
        clr.deinit()

    def load_config(self) -> int:
        self.config = WFCppSampleConfig(self.args.config)
        if not self.config.parse():
            raise Exception("incorrect XML configuration formats!")
        self.config.load(input=self.args.input, names=self.args.name)
        return len(self.config.overlappings) + len(self.config.simpletileds)

    def run(self) -> None:
        success: int = 0
        for sample in self.config.overlappings:
            for i in range(self.args.trial):
                model = OverlappingModel(
                    sample.input, sample.options, sample.seed)
                sample.iter = i + 1
                sample.total_time = -time.time()
                sample.proccess_time = -time.process_time()
                sample.output = model.run()
                sample.total_time += time.time()
                sample.proccess_time += time.process_time()
                if sample.output is None:
                    continue
                success += 1
                break
            print(sample)
            # TODO: check failure
        return success

    def save_out(self) -> None:
        self.config.save_out(self.args.output)

    def deinit(self) -> None:
        if self.args.color:
            self._decolorize()


def _main() -> None:
    app = WFCppCLI()
    print(app.args)
    samples = app.load_config()
    print(samples)
    app.run()
    app.save_out()
    app.deinit()


if __name__ == "__main__":
    # TODO: modulize?
    # TODO: requirements.txt?
    # TODO: error messages?
    _main()
