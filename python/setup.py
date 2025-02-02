import os
import sys
import platform
import re
import shutil
import site
import subprocess
from glob import glob
from typing import TYPE_CHECKING
from pathlib import Path

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext
from setuptools.command.install import install

import numpy as np

if TYPE_CHECKING:
    from typing import Union


# Based on
# https://github.com/galois-advertising/cmake_setup/blob/master/cmake_setup/cmake/__init__.py

SETUP_DIR = Path(__file__).resolve().parent
SOURCE_DIR = SETUP_DIR.parent

# Set platform-specific CMAKE_ARGS
if platform.system() in ["Linux", "Darwin"]:
    dlite_compiled_ext = "_dlite.so"
    dlite_compiled_dll_suffix = "*.so"
    CMAKE_ARGS = [
        "-DWITH_DOC=OFF",
        "-DWITH_HDF5=OFF",
        "-DALLOW_WARNINGS=ON",
        "-Ddlite_PYTHON_BUILD_REDISTRIBUTABLE_PACKAGE=YES",
        # Will always have CMake version >= 3.14 (see `CMakeLists.txt`)
        "-DPython3_FIND_VIRTUALENV=ONLY",
        "-DPython3_FIND_IMPLEMENTATIONS=CPython",
    ]
    if not bool(int(os.getenv("CIBUILDWHEEL", "0"))):
        # Not running with `cibuildwheel`
        CMAKE_ARGS.extend(
            [
                f"-DPython3_EXECUTABLE={sys.executable}",
                "-DCMAKE_INSTALL_PREFIX="
                f"{site.USER_BASE if '--user' in sys.argv else sys.prefix}",
            ]
        )
elif platform.system() == "Windows":
    dlite_compiled_ext = "_dlite.pyd"
    dlite_compiled_dll_suffix = "*.dll"
    is_64bits = sys.maxsize > 2**32
    arch = 'x64' if is_64bits else 'x86'
    v = sys.version_info
    CMAKE_ARGS = [
        #"-G", "Visual Studio 15 2017",
        "-A", "x64",
        "-DWITH_DOC=OFF",
        "-DWITH_HDF5=OFF",
        f"-DPYTHON_VERSION={v.major}.{v.minor}",
        "-Ddlite_PYTHON_BUILD_REDISTRIBUTABLE_PACKAGE=YES",
        f"-DCMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE={arch}",
        "-DPython3_FIND_VIRTUALENV=STANDARD",
    ]
else:
    raise NotImplementedError(f"Unsupported platform: {platform.system()}")


requirements = [
    line.strip() for line in open(SOURCE_DIR/"requirements.txt", "rt")
]
requirements_dev = [
    line.strip() for line in open(SOURCE_DIR/"requirements_dev.txt", "rt")
]


class CMakeExtension(Extension):
    """
    setuptools.Extension for cmake
    """

    def __init__(
        self,
        name: str,
        sourcedir: "Union[Path, str]",
        python_package_dir: "Union[Path, str]",
    ) -> None:
        """
        :param sourcedir: The root directory for the cmake build
        :param python_package_dir: The location of the Python package
            generated by CMake (relative to CMakes build-dir)
        """
        super().__init__(name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)
        self.python_package_dir = python_package_dir


class CMakeBuildExt(build_ext):
    """
    setuptools build_exit which builds using CMake
    """

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)

    def build_extension(self, ext: CMakeExtension) -> None:
        """Run CMAKE

        Extra CMAKE arguments can be added through the `CI_BUILD_CMAKE_ARGS`
        environment variable.
        Note, the variables will be split according to the delimiter, which
        is a comma (`,`).

        Example:

            `CI_BUILD_CMAKE_ARGS="-DWITH_STATIC_PYTHON=YES,-DWITH_HDF5=NO"`
        """

        # The build_temp directory is not generated automatically on Windows,
        # generate it now
        Path(self.build_temp).mkdir(parents=True, exist_ok=True)

        output_dir = os.path.abspath(os.path.dirname(
            self.get_ext_fullpath(ext.name)))

        environment_cmake_args = os.getenv("CI_BUILD_CMAKE_ARGS", "")
        environment_cmake_args = (
            environment_cmake_args.split(",") if environment_cmake_args else []
        )

        # Remove old CMakeCache if it exists in build directory
        cachefile = Path(self.build_temp) / "CMakeCache.txt"
        if cachefile.exists():
            cachefile.unlink()

        # Find cmake executable (using os.path for Python 3.7 compatibility)
        stem, fileext = os.path.splitext(sys.executable)
        cmake_exe = os.path.join(os.path.dirname(stem), "cmake" + fileext)
        if "CMAKE_EXECUTABLE" in os.environ:
            cmake_exe = os.environ["CMAKE_EXECUTABLE"]
        elif not os.path.exists(cmake_exe):
            cmake_exe = shutil.which("cmake" + fileext)

        build_type = "Debug" if self.debug else "Release"
        cmake_args = [
            cmake_exe,
            str(ext.sourcedir),
            f"-DCMAKE_CONFIGURATION_TYPES:STRING={build_type}",
            f"-DPython3_NumPy_INCLUDE_DIRS={np.get_include()}",
        ]
        cmake_args.extend(CMAKE_ARGS)
        cmake_args.extend(environment_cmake_args)

        env = os.environ.copy()

        subprocess.run(
            cmake_args,
            cwd=self.build_temp,
            env=env,
            #capture_output=True,
            check=True,
        )

        subprocess.run(
            ["cmake", "--build", ".", "--config", build_type, "--verbose"],
            cwd=self.build_temp,
            env=env,
            #capture_output=True,
            check=True,
        )

        cmake_bdist_dir = Path(self.build_temp) / Path(ext.python_package_dir)
        shutil.copytree(
            str(cmake_bdist_dir / ext.name),
            str(Path(output_dir) / ext.name),
            dirs_exist_ok=True,
        )


class CustomInstall(install):
    """Custom handler for the 'install' command."""
    def run(self):
        super().run()
        bindir = Path(self.build_lib) / "dlite" / "share" / "dlite" / "bin"
        # Possible to make a symlink instead of copy to save space
        for prog in glob(str(bindir / "*")):
            shutil.copy(prog, self.install_scripts)


version = re.search(
    r"project\([^)]*VERSION\s+([0-9.]+)",
    (SOURCE_DIR / "CMakeLists.txt").read_text(),
).groups()[0]
share = Path(".") / "share" / "dlite"

setup(
    name="DLite-Python",
    version=version,
    author="SINTEF",
    author_email="jesper.friis@sintef.no",
    platforms=["Windows", "Linux"],
    description=(
        "Lightweight data-centric framework for working with scientific data"
    ),
    long_description=(SOURCE_DIR / "README.md").read_text(encoding="utf-8"),
    long_description_content_type="text/markdown",
    url="https://github.com/SINTEF/dlite",
    license="MIT",
    python_requires=">=3.8",
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Intended Audience :: Information Technology",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
        "Operating System :: Microsoft :: Windows",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
        "Topic :: Software Development :: Libraries :: Python Modules",
    ],
    build_requires=requirements_dev,
    install_requires=requirements,
    #extras_require=extra_requirements,
    packages=["DLite-Python"],
    scripts=[
        str(SOURCE_DIR / "bindings" / "python" / "scripts" / "dlite-validate"),
        str(SOURCE_DIR / "cmake" / "patch-activate.sh"),
    ],
    package_data={
        "dlite": [
            str(share / "README.md"),
            str(share / "LICENSE"),
            str(share / "storage-plugins" / dlite_compiled_dll_suffix),
            str(share / "mapping-plugins" / dlite_compiled_dll_suffix),
            str(share / "python-storage-plugins" / "*.py"),
            str(share / "python-mapping-plugins" / "*.py"),
            str(share / "python-protocol-plugins" / "*.py"),
            str(share / "storages" / "*.json"),
            str(share / "storages" / "*.yaml"),
        ]
    },
    ext_modules=[
        CMakeExtension(
            "dlite",
            sourcedir=SOURCE_DIR,
            python_package_dir=Path("bindings") / "python"
        )
    ],
    cmdclass={
        "build_ext": CMakeBuildExt,
        "install": CustomInstall,
    },
    zip_safe=False,
)
