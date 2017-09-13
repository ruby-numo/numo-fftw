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
