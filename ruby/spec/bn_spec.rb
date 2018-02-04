require 'spec_helper'

describe NotSoBignum::Bn do
  Bn = NotSoBignum::Bn

  it 'should support numeric conversion' do
    1_000.times do
      i = Random.rand(1_000_000_000_000)
      expect(Bn.from_i(i).to_i).to eq i
    end
  end

  describe 'operations' do
    class TestBn
      attr_accessor :bn
      attr_accessor :num

      def initialize(args = { :bn => nil, :num => nil })
        @bn = args[:bn]
        @num = args[:num]
      end

      def self.from_i(n)
        new(:bn => Bn.from_i(n), :num => n)
      end

      def check
        raise "fail #{bn.to_i} != #{num}" unless bn.to_i == num
      end

      def +(other)
        puts "#{num} + #{other.num}"
        self.class.new(:bn => bn + other.bn,
                       :num => num + other.num)
      end

      def -(other)
        puts "#{num} - #{other.num}"
        self.class.new(:bn => bn - other.bn,
                       :num => num - other.num)
      end

      def *(other)
        puts "#{num} * #{other.num}"
        self.class.new(:bn => bn * other.bn,
                       :num => num * other.num)
      end

      def /(other)
        puts "#{num} / #{other.num}"
        self.class.new(:bn => bn / other.bn,
                       :num => num / other.num)
      end

      def mod(other)
        puts "#{num} % #{other.num}"
        self.class.new(:bn => bn.mod(other.bn),
                       :num => num % other.num)
      end
    end

    RANDOM_RANGE = 1_000_000_000

    it 'should convert from Integer' do
      Bn.from_i(32).print
    end

    it 'should add' do
      10_000.times do
        x, y = [Random.rand(RANDOM_RANGE), Random.rand(RANDOM_RANGE)]
        a = TestBn.from_i(x)
        b = TestBn.from_i(y)
        c = a + b
        c.check
      end
    end

    it 'should subtract' do
      10_000.times do
        x, y = [Random.rand(RANDOM_RANGE), Random.rand(RANDOM_RANGE)].sort.reverse
        a = TestBn.from_i(x)
        b = TestBn.from_i(y)
        c = a - b
        c.check
      end
    end

    it 'should multiply' do
      10_000.times do
        x, y = [Random.rand(RANDOM_RANGE), Random.rand(RANDOM_RANGE)]
        a = TestBn.from_i(x)
        b = TestBn.from_i(y)
        c = a * b
        c.check
      end
    end

    it 'should divide' do
      10_000.times do
        x, y = [Random.rand(RANDOM_RANGE), Random.rand(RANDOM_RANGE)]
        a = TestBn.from_i(x)
        b = TestBn.from_i(y)
        c = a / b
        c.check
      end
    end

    it 'should remainder' do
      10_000.times do
        x, y = [Random.rand(RANDOM_RANGE), Random.rand(RANDOM_RANGE)]
        a = TestBn.from_i(x)
        b = TestBn.from_i(y)
        c = a.mod(b)
        c.check
      end
    end
  end
end
