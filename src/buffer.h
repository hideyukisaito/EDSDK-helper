#pragma once

#include <iostream>
#include <vector>

namespace eds
{
    class Buffer
    {
    public:
        Buffer();
        Buffer(const char* buffer, unsigned int size);
        Buffer(const std::string& text);
        Buffer(std::istream& stream);
        Buffer(const Buffer& buffer);
        Buffer& operator=(const std::string& text);
        ~Buffer();
        
        void set(const char* buffer, unsigned int size);
        void set(const std::string& text);
        bool set(std::istream& stream);
        void append(const std::string& buffer);
        void append(const char* buffer, unsigned int size);
        
        bool writeTo(std::ostream& stream) const;
        
        void clear();
        
        void allocate(long _size);
        
        char * getBinaryBuffer();
        const char * getBinaryBuffer() const;
        
        std::string getText() const;
        operator std::string() const;  // cast to string, to use a buffer as a string
        
        long size() const;
        std::string getNextLine();
        std::string getFirstLine();
        bool isLastLine();
        void resetLineReader();
        
        friend std::ostream& operator<<(std::ostream& ostr, const Buffer& buffer);
        friend std::istream& operator>>(std::istream& istr, Buffer& buffer);
        
    private:
        std::vector<char> mBuffer;
        long mNextLinePos;
    };
}