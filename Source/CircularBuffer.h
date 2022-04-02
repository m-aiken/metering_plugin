/*
  ==============================================================================

    CircularBuffer.h
    Created: 2 Apr 2022 12:42:57am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

//==============================================================================
template<typename T>
struct CircularBuffer
{
    using DataType = std::vector<T>;
    
    CircularBuffer(size_t numElements, T initialValue)
    {
        resize(numElements, initialValue);
    }
    
    void resize(size_t s, T fillValue) { buffer.resize(s, fillValue); }
    void clear(T fillValue) { buffer.assign(getSize(), fillValue); }
    
    void write(T t)
    {
        auto idx = writeIndex.load();
        buffer[idx] = t;
        
        // increment writeIndex
        ++idx;
        if ( idx > getSize() - 1 ) // end of container, circle back to start
            idx = 0;
        
        writeIndex = idx;
    }
    
    DataType& getData() { return buffer; }
    
    size_t getReadIndex() const
    {
        // writeIndex was incremented in write() so now points to oldest item
        return writeIndex.load();
    }
    
    size_t getSize() const { return buffer.size(); }
    
private:
    DataType buffer;
    std::atomic<int> writeIndex = 0;
};
