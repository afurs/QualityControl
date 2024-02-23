// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file   HelperHist.h
/// \author Artur Furs afurs@cern.ch
/// \brief Helper class for FIT detectors

#ifndef QC_MODULE_FIT_FITHELPER_H
#define QC_MODULE_FIT_FITHELPER_H

#include <map>
#include <set>
#include <string>
#include <utility>
#include <unordered_map>
#include <functional>
#include <array>
#include <vector>
#include <bitset>

#include "TH1D.h"
#include "TH2F.h"

#include <DataFormatsFIT/Triggers.h>
#include "CommonDataFormat/BunchFilling.h"

#include <FITCommon/HelperHist.h>
/*
TODO: all logic below should be moved into O2
*/
namespace o2::quality_control_modules::fit
{
namespace data_helper
{
#define CREATE_FIT_CHDATA_ACCESSOR(Name, Field) \
  template<typename T> \
  constexpr auto Name(T &&channelData) -> std::decay_t<decltype(std::declval<T>().Field)> && \
  { \
    return std::forward<std::decay_t<decltype(std::declval<T>().Field)>>(channelData.mChargeADC); \
  }

  CREATE_FIT_CHDATA_ACCESSOR(amp, mChargeADC)
  CREATE_FIT_CHDATA_ACCESSOR(amp, QTCAmpl)

  CREATE_FIT_CHDATA_ACCESSOR(time, mTime)
  CREATE_FIT_CHDATA_ACCESSOR(time, CFDTime)

  CREATE_FIT_CHDATA_ACCESSOR(pmBits, mFEEBits)
  CREATE_FIT_CHDATA_ACCESSOR(pmBits, ChainQTC)

  CREATE_FIT_CHDATA_ACCESSOR(chID, mPMNumber)
  CREATE_FIT_CHDATA_ACCESSOR(chID, ChId)

#undef CREATE_FIT_CHDATA_ACCESSOR
}
template <typename DigitType, typename ChannelDataType>
class HelperFIT
{
 public:
  using ChannelData_t = ChannelDataType;
  using Digit_t = DigitType;
  typedef decltype(std::declval<Digit_t>().mTriggers) Triggers_t;
  static const inline std::map<unsigned int, std::string> mMapPMbits={
    { ChannelData_t::kNumberADC, "NumberADC" },
    { ChannelData_t::kIsDoubleEvent, "IsDoubleEvent" },
    { ChannelData_t::kIsTimeInfoNOTvalid, "IsTimeInfoNOTvalid" },
    { ChannelData_t::kIsCFDinADCgate, "IsCFDinADCgate" },
    { ChannelData_t::kIsTimeInfoLate, "IsTimeInfoLate" },
    { ChannelData_t::kIsAmpHigh, "IsAmpHigh" },
    { ChannelData_t::kIsEventInTVDC, "IsEventInTVDC" },
    { ChannelData_t::kIsTimeInfoLost, "IsTimeInfoLost" }
  };
};

class HelperTrgFIT
{
 public:
  HelperTrgFIT() = delete;
  ~HelperTrgFIT() = delete;
  static const std::map<unsigned int, std::string> sMapTrgBits;
  static const std::map<unsigned int, std::string> sMapBasicTrgBitsFDD;
  static const std::map<unsigned int, std::string> sMapBasicTrgBitsFT0;
  static const std::map<unsigned int, std::string> sMapBasicTrgBitsFV0;
  static const std::array<std::vector<uint8_t>, 256> sArrDecomposed1Byte;
  inline static std::array<std::vector<uint8_t>, 256> decompose1Byte()
  {
    std::array<std::vector<uint8_t>, 256> arrBitPos{};
    for (int iByteValue = 0; iByteValue < arrBitPos.size(); iByteValue++) {
      auto& vec = arrBitPos[iByteValue];
      for (int iBit = 0; iBit < 8; iBit++) {
        if (iByteValue & (1 << iBit)) {
          vec.push_back(iBit);
        }
      }
    }
    return arrBitPos;
  }
};
} // namespace o2::quality_control_modules::fit
#endif
