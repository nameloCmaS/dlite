"""Some utilities for testing."""
import importlib
import os
import socket
import sys

import dlite


class UnexpectedSuccessError(Exception):
    """Code that was expected to raise an exeption didn't do so."""


class UnexpectedExceptionError(Exception):
    """Code did not raise the expected exception."""


class raises():
    """Assert that a code block raises one of the expected exceptions
    listed in `exceptions`.

    Arguments:
        *exceptions: Expected exception classes.
        silent: Whether to silent DLite error messages in the code block.
    """
    def __init__(self, *exceptions, silent=True):
        self.exceptions = exceptions
        self.silent = silent

    def __enter__(self):
        if self.silent:
            # Silence DLite error messages from expected exceptions and save the
            # current err_ignore state
            self.save_silenced = {}
            for exc in self.exceptions:
                code = dlite._dlite._err_getcode(exc.__name__)
                self.save_silenced[code] = dlite._dlite._err_ignore_get(code)
                dlite._dlite._err_ignore_set(code, 1)

    def __exit__(self, exc_type, exc_value, tb):
        if self.silent:
            # Restore the err_ignore state
            for code, value in self.save_silenced.items():
                dlite._dlite._err_ignore_set(code, value)

        excnames = ", ".join(repr(exc.__name__) for exc in self.exceptions)
        if exc_type is None:
            raise UnexpectedSuccessError(
                f"Expected one of the following exceptions: {excnames}, "
                "but no exception was raised."
            )

        # Leave context manager gracefully if one of the expected exceptions
        # was raised
        for exc in self.exceptions:
            if issubclass(exc_type, exc):
                dlite.errclr()
                return True

        raise UnexpectedExceptionError(
            f"Expected one of the following exceptions: {excnames}, "
            f"but got: '{exc_type.__name__}'"
        ) from exc_value


def importcheck(module_name, package=None):
    """Import and return the requested module or None if the module can't
    be imported."""
    try:
        return importlib.import_module(module_name, package=package)
    except ModuleNotFoundError as exc:
        return None


def importskip(module_name, package=None, exitcode=44,
               env_exitcode="DLITE_IMPORTSKIP_EXITCODE"):
    """Import and return the requested module.

    Calls `sys.exit()` with given exitcode if the module cannot be imported.

    Arguments:
        module_name: Name of module to try to import.
        package: Optional package name that the module might reside in.
        exitcode: The default exit code of `sys.exit()` if the module cannot
            be imported.
        env_exitcode: Name of environment variable containing an exitcode
            to call `sys.exit()` with if the module cannot be imported.
            This overrides `exitcode`.

    Notes:
        If you want to run the tests and get an error if a module
        cannot be imported, set environment variable
        `DLITE_IMPORTSKIP_EXITCODE=1` before running the tests (if you
        run with ctest, set `DLITE_IMPORTSKIP_EXITCODE` at configure
        time).

        For packages that depend on external services like postgresql,
        call `importskip()` with `env_exitcode=None` to skip the test
        regardless of whether `DLITE_IMPORTSKIP_EXITCODE` is set or not.

    """
    try:
        return importlib.import_module(module_name, package=package)
    except ModuleNotFoundError as exc:
        if env_exitcode and env_exitcode in os.environ:
            try:
                exitcode = int(os.environ[env_exitcode])
            except:
                pass
        else:
            print(f"{exc}: skipping test", file=sys.stderr)

        sys.exit(exitcode)


def serverskip(server, port, timeout=2, exitcode=44):
    """Ping a remote server and calls `sys.exit()` with given exitcode if it
    doesn't respond within the timeout."""
    try:
        socket.setdefaulttimeout(timeout)
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((server, port))
    except OSError as exc:
        print(
            f"Server {server}:{port} seems to be down: {exc}", file=sys.stderr
        )
        sys.exit(exitcode)
    else:
        s.close()
