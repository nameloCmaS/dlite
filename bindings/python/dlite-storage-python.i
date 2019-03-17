/* -*- c -*-  (not really, but good for syntax highlighting) */

/* Python-spesific extensions to dlite-storage.i */

%extend _DLiteStorage {

  %pythoncode %{
      def __enter__(self):
          return self

      def __exit__(self, *exc):
          # The storage is closed when the corresponding Python object
          # is garbage collected - hence, no need to do anything here
          pass

      def __repr__(self):
          options = '?%s' % self.options if self.options else ''
          return "Storage('%s://%s%s')" % (self.driver, self.uri, options)

      driver = property(get_driver,
                        doc='Name of driver associated with this storage')
  %}
}