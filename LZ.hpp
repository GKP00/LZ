#pragma once

#include <string_view>
#include <tuple>
#include <vector>
#include <cassert>

namespace _detail
{

std::string_view findLongestMatch(const char* searchBegin, const char* lookaheadBegin, const char* lookaheadEnd)
{
  assert(searchBegin <= lookaheadBegin);
  assert(lookaheadBegin <= lookaheadEnd);

  size_t currLen = 0;
  size_t longestLen = 0;
  const char* longestMatch = nullptr;

  auto updateLongest = [&](const char* strEnd)
  {
    if(currLen == 0)
      return;

    if(currLen >= longestLen)
    {
      longestLen = currLen;
      longestMatch = strEnd - currLen;
    }

    currLen = 0;
  };

  const char* sCurs = searchBegin;
  const char* lCurs = lookaheadBegin;

  while(lCurs <= lookaheadEnd && searchBegin < lookaheadBegin)
  {
    if(*sCurs != *lCurs)
    {
      //update longest & skip
      updateLongest(sCurs);

      //cut first byte of search buffer
      ++searchBegin;

      //reset cursors
      lCurs = lookaheadBegin;
      sCurs = searchBegin;
      continue;
    }

    ++currLen;
    ++lCurs;
    ++sCurs;
  }

  updateLongest(sCurs);

  return {longestMatch, longestLen};
}

} //namespace: _detail

namespace LZ77
{

using Token = std::tuple<unsigned short, unsigned short, char>;

std::vector<Token> 
Compress(std::string_view input, size_t searchSize=32768, size_t lookaheadSize=1024)
{
  std::vector<Token> compressed;

  //slide window across input
  for(size_t i = 0; i < input.size(); ++i)
  {
    //Pointers that define the search & lookahead buffers
    //
    //"foobarbazfoobarbazfoobarbazfoobarbazfoobarbazfoobarbaz"
    //      |              |                   |
    //  searchBegin   lookaheadBegin      lookaheadEnd
    const char* searchBegin    = (input.data() + i) - std::min(searchSize, i);
    const char* lookaheadBegin = (input.data() + i);
    const char* lookaheadEnd   = (input.data() + i) + std::min(lookaheadSize, (input.size()-1) - i);

    auto match = 
      _detail::findLongestMatch(searchBegin, lookaheadBegin, lookaheadEnd);

    if(match.data())
    {
      compressed.emplace_back(
          lookaheadBegin - match.data(), 
          match.length(), 
          *(lookaheadBegin + match.length()) );

      i += match.length();
      continue;
    }

    compressed.emplace_back(0, 0, *lookaheadBegin);
  }

  return compressed;
}

std::vector<unsigned char>
Decompress(const std::vector<Token>& compressed)
{
  std::vector<unsigned char> decompressed;

  for(const Token& token : compressed)
  {
    auto pos = std::get<0>(token);
    auto len = std::get<1>(token);
    auto lit = std::get<2>(token);

    if(!pos)
    {
      decompressed.push_back(lit);
      continue;
    }

    auto decompressedSize = decompressed.size();

    for(auto i = 0u; i < len; ++i)
      decompressed.push_back( decompressed[ decompressedSize - pos + (i%len) ] );

    decompressed.push_back(lit);
  }

  return decompressed;
}

} //namespace: LZ77
namespace LZ1 = LZ77;

