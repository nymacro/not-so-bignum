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

    binary_op :BN_add, :+
    binary_op :BN_sub, :-
    binary_op :BN_mul, :*
    binary_op :BN_div, :/
  end
end
