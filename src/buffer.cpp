#include "buffer.h"

namespace eds
{
    Buffer::Buffer()
    {
        mNextLinePos = 0;
        mBuffer.resize(1);
    }
    
    Buffer::Buffer(const char* buffer, unsigned int size)
    {
        set(buffer, size);
    }
    
    Buffer::Buffer(const std::string& text)
    {
        set(text);
    }
    
    Buffer::Buffer(std::istream& stream)
    {
        set(stream);
    }
    
    Buffer::Buffer(const Buffer& buffer)
    {
        mBuffer = buffer.mBuffer;
        mNextLinePos = buffer.mNextLinePos;
    }
    
    Buffer& Buffer::operator=(const std::string & text)
    {
        set(text);
        return *this;
    }
    
    Buffer::~Buffer()
    {
        clear();
    }
    
    bool Buffer::set(std::istream& stream)
    {
        clear();
        
        if(stream.bad())
        {
            return false;
        }
        
        char temp_[1024];
        
        auto size = 0;
        stream.read(temp_, 1024);
        
        auto n = stream.gcount();
        
        while (n > 0)
        {
            // we resize to size+1 initialized to 0 to have a 0 at the end for strings
            mBuffer.resize(size + n + 1, 0);
            memcpy(&(mBuffer[0]) + size, temp_, n);
            
            size += n;
            
            if (stream)
            {
                stream.read(temp_, 1024);
                n = stream.gcount();
            }
            else
            {
                n = 0;
            }
        }
        return true;
    }
    
    bool Buffer::writeTo(std::ostream & stream) const
    {
        if(stream.bad())
        {
            return false;
        }
        
        stream.write(&(mBuffer[0]), mBuffer.size() - 1);
        
        return true;
    }
    
    void Buffer::set(const char* buffer, unsigned int size)
    {
        mBuffer.assign(buffer, buffer + size);
        mBuffer.resize(mBuffer.size() + 1);
        mBuffer.back() = 0;
    }
    
    void Buffer::set(const std::string& text)
    {
        set(text.c_str(), text.size());
    }
    
    void Buffer::append(const std::string& buffer)
    {
        append(buffer.c_str(), buffer.size());
    }
    
    void Buffer::append(const char * buffer, unsigned int size)
    {
        mBuffer.insert(mBuffer.end() - 1, buffer, buffer + size);
        mBuffer.back() = 0;
    }
    
    void Buffer::clear()
    {
        mBuffer.resize(1);
        mNextLinePos = 0;
    }
    
    void Buffer::allocate(long size)
    {
        clear();
        mBuffer.resize(size);
    }
    
    char* Buffer::getBinaryBuffer()
    {
        if(mBuffer.empty())
        {
            return NULL;
        }
        
        return &mBuffer[0];
    }
    
    const char* Buffer::getBinaryBuffer() const
    {
        if(mBuffer.empty())
        {
            return "";
        }
        
        return &mBuffer[0];
    }
    
    std::string Buffer::getText() const
    {
        if(mBuffer.empty())
        {
            return "";
        }
        
        return &mBuffer[0];
    }
    
    Buffer::operator std::string() const
    {
        return getText();
    }
    
    long Buffer::size() const
    {
        if(mBuffer.empty())
        {
            return 0;
        }
        
        //we always add a 0 at the end to avoid problems with strings
        return mBuffer.size() - 1;
    }
    
    std::string Buffer::getNextLine()
    {
        if(mBuffer.empty() || (int)(mBuffer.size() - 1) == mNextLinePos)
        {
            return "";
        }
        
        auto currentLinePos_ = mNextLinePos;
        auto lineEndWasCR_ = false;
        
        while(mNextLinePos < (int)mBuffer.size() - 1 && mBuffer[mNextLinePos] != '\n')
        {
            if(mBuffer[mNextLinePos] != '\r')
            {
                mNextLinePos++;
            }
            else
            {
                lineEndWasCR_ = true;
                break;
            }
        }
        
        std::string line(getBinaryBuffer() + currentLinePos_, mNextLinePos - currentLinePos_);
        
        if(mNextLinePos < (int)(mBuffer.size() - 1))
        {
            mNextLinePos++;
        }
        
        // if lineEndWasCR check for CRLF
        if(lineEndWasCR_ && mNextLinePos < (int)(mBuffer.size() - 1) && mBuffer[mNextLinePos] == '\n')
        {
            mNextLinePos++;
        }
        
        return line;
    }
    
    std::string Buffer::getFirstLine()
    {
        resetLineReader();
        return getNextLine();
    }
    
    bool Buffer::isLastLine()
    {
        return (int)(mBuffer.size() - 1) == mNextLinePos;
    }
    
    void Buffer::resetLineReader()
    {
        mNextLinePos = 0;
    }
    
    std::ostream& operator<<(std::ostream& ostr, const Buffer & buffer)
    {
        buffer.writeTo(ostr);
        return ostr;
    }
    
    std::istream& operator>>(std::istream& istr, Buffer& buffer)
    {
        buffer.set(istr);
        return istr;
    }
}