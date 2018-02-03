require 'ffi'

module NotSoBignum
  module C
    extend FFI::Library

    ffi_lib File.join(File.dirname(__FILE__), '..', '..', '..', 'libnbn.so')

    attach_function :BN_new, [], :pointer
    attach_function :BN_free, [:pointer], :void

    attach_function :BN_inc, [:pointer], :void
    attach_function :BN_dec, [:pointer], :void

    attach_function :BN_add, [:pointer, :pointer, :pointer], :void
    attach_function :BN_sub, [:pointer, :pointer, :pointer], :void
    attach_function :BN_mul, [:pointer, :pointer, :pointer], :void
    attach_function :BN_div, [:pointer, :pointer, :pointer], :void
    attach_function :BN_shl_u64, [:pointer, :pointer, :uint64], :void
    attach_function :BN_shr_u64, [:pointer, :pointer, :uint64], :void

    attach_function :BN_from_hex, [:pointer, :string], :int
    attach_function :BN_new_from_hex, [:string], :pointer

    attach_function :BN_print, [:pointer], :void
    attach_function :BN_to_string, [:pointer, :buffer_out, :size_t], :int

    attach_function :BN_zero, [], :pointer
    attach_function :BN_one, [], :pointer
    attach_function :BN_two, [], :pointer
    attach_function :BN_three, [], :pointer
    attach_function :BN_four, [], :pointer
    attach_function :BN_five, [], :pointer
    attach_function :BN_six, [], :pointer
    attach_function :BN_seven, [], :pointer
    attach_function :BN_eight, [], :pointer
    attach_function :BN_nine, [], :pointer
    attach_function :BN_ten, [], :pointer
  end
end
