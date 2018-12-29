#pragma once

#include <algorithm>
#include "Assert.h"

namespace core{

#define BYTE_BIT_COUNT 8
    
    template<std::size_t SymbolSize>
    class Symbol
    {
    public:
        Symbol(char* const symbolBuffer, int firstByteOffset)
            :m_buffer(symbolBuffer), m_firstByteOffset(firstByteOffset)
        {
            m_byteCount = SymbolSize % BYTE_BIT_COUNT == 0 ? SymbolSize / BYTE_BIT_COUNT : SymbolSize / BYTE_BIT_COUNT + 1;
            m_byteCount += m_firstByteOffset + SymbolSize > BYTE_BIT_COUNT && //Does the symbol crosses the first byte?
                    (m_firstByteOffset + SymbolSize) % BYTE_BIT_COUNT != 0 ? 1 : 0; //Symbol crosses byte boundaries at the last byte?
            static_assert((sizeof(long) * BYTE_BIT_COUNT) >= SymbolSize, "Symbol size is too large");
        }
        
        void operator = (long value)
        {
            int bitOffset = 0;
            char* valueByte = reinterpret_cast<char*>(&value);
            for(int byteIdx = 0; byteIdx < m_byteCount; byteIdx++)
            {
                int byteOffset = byteIdx == 0 ? m_firstByteOffset : 0;
                int currentByteBitCount = byteIdx == 0 ? BYTE_BIT_COUNT - m_firstByteOffset : std::min(BYTE_BIT_COUNT, ((int)SymbolSize) - bitOffset);
                valueByte += (bitOffset / BYTE_BIT_COUNT);
                char* currentByte = m_buffer + byteIdx;
                for(int byteBitIdx = 0; byteBitIdx < currentByteBitCount; byteBitIdx++)
                {
                    char byte = *valueByte & (0x1 << (bitOffset % BYTE_BIT_COUNT));
                    if(byte)
                        *currentByte = *currentByte | 0x1 << (byteBitIdx + byteOffset) ;
                    else
                        *currentByte = *currentByte & ~(0x1 << (byteBitIdx + byteOffset));
                    bitOffset++;
                    if(bitOffset == SymbolSize)
                        break;
                }
            }
        }
        
        operator long()
        {
            VERIFY(sizeof(long) >= SymbolSize / BYTE_BIT_COUNT, "long is not large enough to support the symbol");
            return ToIntegralType<long>();
        }
    
        operator int()
        {
            VERIFY(sizeof(int) >= SymbolSize / BYTE_BIT_COUNT, "int is not large enough to support the symbol");
            return ToIntegralType<int>();
        }
        
        operator short()
        {
            VERIFY(sizeof(short) >= SymbolSize / BYTE_BIT_COUNT, "short is not large enough to support the symbol");
            return ToIntegralType<short>();
        }
    
        operator char()
        {
            VERIFY(sizeof(char) >= SymbolSize * BYTE_BIT_COUNT, "char is not large enough to support the symbol");
            return ToIntegralType<char>();
        }
    
        template<typename IntegralType>
        IntegralType ToIntegralType()
        {
    
            IntegralType result = 0;
            int bitOffset = 0;
            char* resultByte = reinterpret_cast<char*>(&result);
            for(int byteIdx = 0; byteIdx < m_byteCount; byteIdx++)
            {
                int byteOffset = byteIdx == 0 ? m_firstByteOffset : 0;
                int currentByteBitCount = byteIdx == 0 ? BYTE_BIT_COUNT - m_firstByteOffset : std::min(BYTE_BIT_COUNT, ((int)SymbolSize) - bitOffset);
                resultByte += (bitOffset / BYTE_BIT_COUNT);
                char bufferByte = *(m_buffer + byteIdx);
                for(int byteBitIdx = 0; byteBitIdx < currentByteBitCount; byteBitIdx++)
                {
                    *resultByte = *resultByte | (((bufferByte & (0x1 << (byteOffset + byteBitIdx))) >> byteOffset) << bitOffset);
                    bitOffset++;
                    if(bitOffset == SymbolSize)
                        break;
                }
            }
            return result;
        }
        
    private:
        char* const m_buffer;
        std::size_t m_byteCount;
        int m_firstByteOffset;
    };
    
    template<std::size_t SymbolSize, std::size_t NumOfSymbols>
    class Symbolset
    {
    private:
        const int ByteCount = (SymbolSize / BYTE_BIT_COUNT) * NumOfSymbols;
    public:
        Symbolset()
        {
            int totalBitCount = SymbolSize * NumOfSymbols;
            int byteCount = totalBitCount % BYTE_BIT_COUNT == 0 ? totalBitCount / BYTE_BIT_COUNT :
                            (totalBitCount / BYTE_BIT_COUNT) + 1;
            m_buffer = new char[byteCount];
            memset(m_buffer, 0, ByteCount);
        }
        Symbolset(long defaultValue)
        {
            int totalBitCount = SymbolSize * NumOfSymbols;
            int byteCount = totalBitCount % BYTE_BIT_COUNT == 0 ? totalBitCount / BYTE_BIT_COUNT :
                            (totalBitCount / BYTE_BIT_COUNT) + 1;
            m_buffer = new char[byteCount];
            for(int symbolIdx = 0; symbolIdx < NumOfSymbols; symbolIdx++)
            {
                operator[](symbolIdx) = defaultValue;
            }
        }
        
        ~Symbolset(){ delete [] m_buffer; }
        
        Symbol<SymbolSize> operator[](std::size_t index)
        {
            int byteCount = static_cast<int>(index * SymbolSize) / BYTE_BIT_COUNT;
            int bitOffset =  (int)(index * SymbolSize) % BYTE_BIT_COUNT;
            return Symbol<SymbolSize>(m_buffer + byteCount, bitOffset);
        }
    
    private:
        char* m_buffer;
    };
}
