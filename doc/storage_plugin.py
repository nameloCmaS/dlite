"""Template for Python storage plugins."""


class plugin_driver_name(dlite.DLiteStorageBase):
    """General description of the Python storage plugin."""

    def open(self, location, options=None):
        """Open storage.

        Arguments:
            location: Path to storage.
            options: Additional options for this storage driver.
        """

    def close(self):
        """Close the storage.  Optional.
        This will automatically call flush() if flush is defined.
        """

    def flush(self):
        """Flush cached data to the storage.  Optional."""

    def load(self, id=None):
        """Load an instance from storage and return it.  Optional.

        Arguments:
            id: ID of instance to load from the storage.

        Returns:
            New instance.
        """

    def save(self, inst):
        """Save instance `inst` to storage.  Optional.

        Arguments:
            inst: Instance to save.
        """

    def delete(self, id):
        """Delete instance with given `id` from storage.  Optional.

        Arguments:
            id: URI or UUID of instance to delete.
        """

    def queue(self, pattern=None):
        """Generator method that iterates over all UUIDs in the storage
        who"s metadata URI matches glob pattern `pattern`.

        Arguments:
            pattern: Glob pattern for matching metadata URIs.

        Yields:
            Instance UUIDs mabased on the `pattern` regular expression.
            If no `pattern` is given, the UUIDs of all instances in the
            storage are yielded.
        """

    @classmethod
    def from_bytes(cls, buffer, id=None):
        """Load instance with given `id` from `buffer`.

        Arguments:
            buffer: Bytes or bytearray object to load the instance from.
            id: ID of instance to load.  May be omitted if `buffer` only
                holds one instance.

        Returns:
            New instance.
        """

    @classmethod
    def to_bytes(cls, inst):
        """Save instance `inst` to bytes (or bytearray) object.  Optional.

        Arguments:
            inst: Instance to save.

        Returns:
            The bytes (or bytearray) object that the instance is saved to.
        """
