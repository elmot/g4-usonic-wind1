//
// Created by Ilia.Motornyi on 11/04/2020.
//

extern "C" {
#include "main.h"
}
#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>

using namespace std;

static const int extremum_find_range = 5;
static const int SRC_DATA_LEN = 30000;
static const int INTER_DATA_LEN = SRC_DATA_LEN / 10;

typedef array<uint16_t, ADC_BUF_LEN> &SrcDataType;

template<size_t Len>
class InterData {
  array<tuple<unsigned int, unsigned int>, Len> data{};
  typename array<tuple<unsigned int, unsigned int>, Len>::pointer endPtr = data.begin();
  enum Direction { UP,
    DOWN
  };
  Direction calcDir(SrcDataType &srcData, int i) {
    return srcData[i] - srcData[i + extremum_find_range] > 0 ? DOWN : UP;
  }

public:
  typename array<tuple<unsigned int, unsigned int>, Len>::iterator begin() {
    return data.begin();
  }
  std::size_t size() {
    return std::distance(end(), begin());
  }
  typename array<tuple<unsigned int, unsigned int>, Len>::iterator end() {
    return endPtr;
  }

  void  calc(SrcDataType &srcData) {
    unsigned long sum = srcData[0];
    Direction dir = calcDir(srcData, 0);
    for (int i = 1; (i < srcData.size() - extremum_find_range); ++i) {
      sum += srcData[i];
      Direction newDir = calcDir(srcData, i);
      if (dir != newDir) {
        int middlePoint = srcData[i + extremum_find_range / 2];
        int x = i;
        uint16_t y = middlePoint;
        *endPtr++ = {x, y};
        dir = newDir;
        if(endPtr == data.end()) {
          break;
        }
      }
    }
    auto average = sum / (srcData.size() - 1 - extremum_find_range);

    for (auto foundPoint = data.begin(); foundPoint != endPtr; foundPoint++) {
      if (get<1>(*foundPoint) > average) {
        get<1>(*foundPoint) = get<1>(*foundPoint) - average;
      } else {
        get<1>(*foundPoint) = average - get<1>(*foundPoint);
      }
    }
  }
};

SrcDataType srcData = reinterpret_cast<SrcDataType &>(adcBuffer);
InterData<INTER_DATA_LEN> foundPoints;

void processData() {
  foundPoints.calc(srcData);
  for (  auto &p : foundPoints ) {
    cout << get<0>(p) <<"\t"<< get<1>(p) << endl;
  }
  __NOP();
  __NOP();
}
