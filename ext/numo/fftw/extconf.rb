require 'rbconfig.rb'
require 'numo/narray'
require 'mkmf'

dir_config("fftw")

#$CFLAGS="-g -O0"
#$INCFLAGS = "-I../ext -I../ext/types #$INCFLAGS"

$LOAD_PATH.each do |x|
  if File.exist? File.join(x,'numo/numo/narray.h')
    $INCFLAGS = "-I#{x}/numo " + $INCFLAGS
    break
  end
end

if !have_header('numo/narray.h')
  print <<EOL
  Header numo/narray.h was not found. Give pathname as follows:
  % ruby extconf.rb --with-narray-include=narray_h_dir
EOL
  exit(1)
end

if /cygwin|mingw/ =~ RUBY_PLATFORM
  narray_include = File.expand_path(File.dirname(Gem.find_files("numo/numo/narray.h")[0]))
  narray_lib = File.expand_path(File.dirname(Gem.find_files("numo/libnarray.a")[0]))
  dir_config('narray', narray_include, narray_lib)

  if !have_library('narray')
    print <<EOL
    Library libnarray.a was not found. Give pathname as follows:
    % ruby extconf.rb --with-narray-lib=library_dir
EOL
    exit(-1)
  end

end

if !have_header('fftw3.h')
  print <<EOL
  Header fftw3.h was not found. Give pathname as follows:
  % ruby extconf.rb --with-fftw-include=header_dir
EOL
  exit(1)
end

if !have_library('fftw3')
  print <<EOL
  Library libfftw3.so was not found. Give pathname as follows:
  % ruby extconf.rb --with-fftw-lib=library_dir
EOL
  exit(1)
end

$objs = %w[fftw.o]

create_makefile('numo/fftw')
