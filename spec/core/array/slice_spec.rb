require File.dirname(__FILE__) + '/../../spec_helper'
require File.dirname(__FILE__) + '/fixtures/classes'
require File.dirname(__FILE__) + '/shared/slice'

describe "Array#slice!" do
  it "removes and return the element at index" do
    a = [1, 2, 3, 4]
    a.slice!(10).should == nil
    a.should == [1, 2, 3, 4]
    a.slice!(-10).should == nil
    a.should == [1, 2, 3, 4]
    a.slice!(2).should == 3
    a.should == [1, 2, 4]
    a.slice!(-1).should == 4
    a.should == [1, 2]
    a.slice!(1).should == 2
    a.should == [1]
    a.slice!(-1).should == 1
    a.should == []
    a.slice!(-1).should == nil
    a.should == []
    a.slice!(0).should == nil
    a.should == []
  end
  
  it "removes and returns length elements beginning at start" do
    a = [1, 2, 3, 4, 5, 6]
    a.slice!(2, 3).should == [3, 4, 5]
    a.should == [1, 2, 6]
    a.slice!(1, 1).should == [2]
    a.should == [1, 6]
    a.slice!(1, 0).should == []
    a.should == [1, 6]
    a.slice!(2, 0).should == []
    a.should == [1, 6]
    a.slice!(0, 4).should == [1, 6]
    a.should == []
    a.slice!(0, 4).should == []
    a.should == []
  end

  it "calls to_int on start and length arguments" do
    obj = Object.new
    def obj.to_int() 2 end
      
    a = [1, 2, 3, 4, 5]
    a.slice!(obj).should == 3
    a.should == [1, 2, 4, 5]
    a.slice!(obj, obj).should == [4, 5]
    a.should == [1, 2]
    a.slice!(0, obj).should == [1, 2]
    a.should == []
    
    obj = Object.new
    obj.should_receive(:respond_to?).with(:to_int).any_number_of_times.and_return(true)
    obj.should_receive(:method_missing).with(:to_int).any_number_of_times.and_return(2)
    a = [1, 2, 3, 4, 5]
    a.slice!(obj).should == 3    
  end

  it "removes and return elements in range" do
    a = [1, 2, 3, 4, 5, 6, 7, 8]
    a.slice!(1..4).should == [2, 3, 4, 5]
    a.should == [1, 6, 7, 8]
    a.slice!(1...3).should == [6, 7]
    a.should == [1, 8]
    a.slice!(-1..-1).should == [8]
    a.should == [1]
    a.slice!(0...0).should == []
    a.should == [1]
    a.slice!(0..0).should == [1]
    a.should == []
  end
  
  it "calls to_int on range arguments" do
    from = Object.new
    to = Object.new
    
    # So we can construct a range out of them...
    def from.<=>(o) 0 end
    def to.<=>(o) 0 end

    def from.to_int() 1 end
    def to.to_int() -2 end
      
    a = [1, 2, 3, 4, 5]
      
    a.slice!(from .. to).should == [2, 3, 4]
    a.should == [1, 5]
  
    lambda { a.slice!("a" .. "b")  }.should raise_error(TypeError)
    lambda { a.slice!(from .. "b") }.should raise_error(TypeError)
    
    from = Object.new
    to = Object.new
    
    def from.<=>(o) 0 end
    def to.<=>(o) 0 end
      
    from.should_receive(:respond_to?).with(:to_int).any_number_of_times.and_return(true)
    from.should_receive(:method_missing).with(:to_int).any_number_of_times.and_return(1)
    to.should_receive(:respond_to?).with(:to_int).any_number_of_times.and_return(true)
    to.should_receive(:method_missing).with(:to_int).any_number_of_times.and_return(-2)

    a = [1, 2, 3, 4, 5]
    a.slice!(from .. to).should == [2, 3, 4]
  end
  
  # TODO: MRI behaves inconsistently here. I'm trying to find out what it should
  # do at ruby-core right now. -- flgr
  # See http://groups.google.com/group/ruby-core-google/t/af70e3d0e9b82f39
  it "does (not?) expand array with indices out of bounds" do
    # This is the way MRI behaves -- subject to change
    a = [1, 2]
    a.slice!(4).should == nil
    a.should == [1, 2]
    a.slice!(4, 0).should == nil
    a.should == [1, 2, nil, nil]
    a.slice!(6, 1).should == nil
    a.should == [1, 2, nil, nil, nil, nil]
    a.slice!(8...8).should == nil
    a.should == [1, 2, nil, nil, nil, nil, nil, nil]
    a.slice!(10..10).should == nil
    a.should == [1, 2, nil, nil, nil, nil, nil, nil, nil, nil]
  end

  compliant :mri do
    it "raises TypeError on a frozen array" do
      lambda { ArraySpecs.frozen_array.slice!(0, 0) }.should raise_error(TypeError)
    end
  end
end

describe "Array#slice" do
  it_behaves_like(:array_slice, :slice)
end
