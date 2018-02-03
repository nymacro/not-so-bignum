require 'pry'
module NotSoBignum
  class Bn
    attr_reader :ptr

    class <<self
      def binary_op(name, rename = name)
        define_method(rename) do |other|
          self.class.new.tap do |bn|
            C.send(name, bn.ptr, ptr, other.ptr)
          end
        end
      end
    end

    def initialize
      @ptr = FFI::AutoPointer.new(C.BN_new,
                                  C.method(:BN_free))
    end

    def from_hex(str)
      C.BN_from_hex(ptr, str)
    end

    def self.from_hex(str)
      new.tap do |bn|
        bn.from_hex(str)
      end
    end

    def to_hex
      buf = FFI::MemoryPointer.new(:uchar, 256)
      len = C.BN_to_string(ptr, buf, buf.size)
      buf.read_bytes(len)
    end

    def to_i
      to_hex.to_i(16)
    end

    def from_i(i)
      from_hex('%016x' % i)
    end

    def self.from_i(i)
      new.tap do |bn|
        bn.from_i(i)
      end
    end

    def print
      C.BN_print(ptr)
    end

    def to_s
      to_hex
    end

    binary_op :BN_add, :+
    binary_op :BN_sub, :-
    binary_op :BN_mul, :*
    binary_op :BN_div, :/
  end
end
