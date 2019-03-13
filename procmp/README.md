Do not optimize duplicate\_fd\_content with mmap() or tee() - they give none to
minimal improvement over read() and write(). Also, mmap() can be only used for
objects with known length (files), and tee() only for pipes.

Example implementation can be found in
duplicate\_pipe\_content\_\*.cpp

