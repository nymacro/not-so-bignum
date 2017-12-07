require 'spec_helper'

describe NotSoBignum::Bn do
  Bn = NotSoBignum::Bn

  it 'should add' do
    a = Bn.from_hex("ff")
    b = Bn.from_hex("ff")
    c = a + b
    c.print
  end

  it 'should convert from Integer' do
    Bn.from_i(32).print
  end
end
