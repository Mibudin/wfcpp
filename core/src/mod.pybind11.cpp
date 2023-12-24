#include <stdexcept>
#include <algorithm>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/attr.h>
#include <pybind11/numpy.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include "utils/color.hpp"
#include "overlapping_model.hpp"


PYBIND11_MODULE(wfcpp, m)
{
    namespace py = pybind11;
    using namespace pybind11::literals;

    using namespace wfcpp;


    m.attr("__version__") = "1.0";
    m.doc() =
        "WFCpp is a simple library for Wave Function Collapse implemented "
        "with modern C++ and also provides interfaces to Python with "
        "pybind11.";


    py::class_<Color>(m, "Color")
        .def(py::init<>())
        .def(py::init<unsigned char, unsigned char, unsigned char>(),
             "r"_a, "g"_a, "b"_a)
        .def_readwrite("r", &Color::r)
        .def_readwrite("g", &Color::g)
        .def_readwrite("b", &Color::b)
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::class_<Array2D<Color>>(m, "ColorMap", py::buffer_protocol())
        .def(py::init<std::size_t, std::size_t>(),
             "height"_a, "width"_a)
        .def(py::init<std::size_t, std::size_t, Color>(),
             "height"_a, "width"_a, "value"_a)
        .def(py::init([](py::buffer buf) -> Array2D<Color>
        {
            py::buffer_info info = buf.request();
            if (info.format != py::format_descriptor<unsigned char>::format())
            {
                throw std::runtime_error(
                    "Incompatible format: expected a uint8 array!");
            }
            if (info.ndim != 3)
            {
                throw std::runtime_error("Incompatible buffer dimension!");
            }

            Array2D<Color> array = Array2D<Color>(info.shape.at(0), info.shape.at(1));
            unsigned char* p = static_cast<unsigned char*>(info.ptr);
            for(Color& c : array.buffer())
            {
                c = {*p, *(p + 1), *(p + 2)};
                p += 3;
            }

            return array;
        }), "buf"_a)
        .def_buffer([](Array2D<Color>& self) -> py::buffer_info
        {
            // Extract `Color` as 3 unsigned char values
            return py::buffer_info(
                reinterpret_cast<unsigned char*>(self.buffer().data()),
                sizeof(unsigned char),
                py::format_descriptor<unsigned char>::format(),
                3,
                {self.height(), self.width(), static_cast<std::size_t>(3)},
                {
                    sizeof(unsigned char) * 3 * self.width(),
                    sizeof(unsigned char) * 3,
                    sizeof(unsigned char)
                }
            );
        })
        .def_property_readonly("height", &Array2D<Color>::height)
        .def_property_readonly("width", &Array2D<Color>::width)
        // .def_property_readonly("data",
        //     py::overload_cast<>(&Array2D<Color>::buffer, py::const_))
        .def(py::self == py::self)
        .def("__getitem__",
            [](Array2D<Color>& self, std::tuple<std::size_t, std::size_t> idx)
                -> Color
            {
                return self(std::get<0>(idx), std::get<1>(idx));
            }
        )
        .def("__setitem__",
            [](Array2D<Color>& self, std::tuple<std::size_t, std::size_t> idx,
                Color value) -> Color
            {
                return self(std::get<0>(idx), std::get<1>(idx)) = value;
            }
        );

    py::class_<OverlappingModelOptions>(m, "OverlappingModelOptions")
        .def(py::init<>())
        .def(py::init(
            [](
               unsigned int pattern_size,
               bool periodic_input,
               bool periodic_output,
               std::size_t out_height,
               std::size_t out_width,
               unsigned int symmetry,
               std::size_t ground_num) -> OverlappingModelOptions
            {
                return {pattern_size, periodic_input, periodic_output,
                        out_height, out_width, symmetry, ground_num != 0};
            }),
            // The following names follow the XML configuration formats
            "N"_a,                      // "pattern_size"_a,
            "periodicInput"_a = true,   // "periodic_input"_a = true,
            "periodic"_a = false,       // "periodic_output"_a = false,
            "height"_a = 48,            // "out_height"_a = 48,
            "width"_a = 48,             // "out_width"_a = 48,
            "symmetry"_a = 8,           // "symmetry"_a = 8,
            "ground"_a = 0              // "ground"_a = false
        )
        .def_readwrite("periodic_input",
                       &OverlappingModelOptions::periodic_input)
        .def_readwrite("periodic_output",
                       &OverlappingModelOptions::periodic_output)
        .def_readwrite("out_height", &OverlappingModelOptions::out_height)
        .def_readwrite("out_width", &OverlappingModelOptions::out_width)
        .def_readwrite("symmetry", &OverlappingModelOptions::symmetry)
        .def_readwrite("ground", &OverlappingModelOptions::ground)
        .def_readwrite("pattern_size", &OverlappingModelOptions::pattern_size);

    py::class_<OverlappingModel<Color>>(m, "OverlappingModel")
        .def(py::init<const Array2D<Color>&, const OverlappingModelOptions&,
                      int>(),
             "input"_a, "options"_a, "seed"_a)
        .def("run", &OverlappingModel<Color>::run);

    m.def("bytes", &CustomAllocator<Color>::bytes);
    m.def("allocated", &CustomAllocator<Color>::allocated);
    m.def("deallocated", &CustomAllocator<Color>::deallocated);
}
