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
/// \brief Trigger validation for FIT detectors

#ifndef QC_MODULE_FIT_TRGVALIDATION_H
#define QC_MODULE_FIT_TRGVALIDATION_H

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
#include <FITCommon/HelperFIT.h>

/*
TODO: all logic below should be moved into O2
*/
namespace o2::quality_control_modules::fit
{

template <typename DigitType, typename ChannelDataType>
struct DataTCM {
  using Digit_t = DigitType;
  typedef decltype(std::declval<Digit_t>().mTriggers) Triggers_t;
  DataTCM(int nPMA, int nPMC): mVecAmpA_PM(nPMA),mVecAmpC_PM(nPMC) {}
  // Copied from o2::fit::Triggers
  //  will be moved there soon
  uint8_t triggersignals{}; // FIT trigger signals
  uint8_t nChanAmpA{};      // number of fired channels A side, for amp calc
  uint8_t nChanAmpC{};      // number of fired channels C side, for amp calc
  uint8_t nChanTimeA{};     // number of fired channels A side, for time calc
  uint8_t nChanTimeC{};     // number of fired channels C side, for time calc

  int32_t amplA{};          // sum amplitude A side
  int32_t amplC{};          // sum amplitude C side
  int16_t timeA{};          // average time A side (shouldn't be used if nChanA == 0)
  int16_t timeC{};          // average time C side (shouldn't be used if nChanC == 0)
  std::vector<int> mVecAmpA_PM{};
  std::vector<int> mVecAmpC_PM{};
  int32_t amplSum{};
  uint8_t nChanSum{};

  inline void fillAmpSideA(const ChannelData_t &channelData, int idxPM)
  {
    mVecAmpA_PM[idxPM]+=data_helper::amp(channelData);
    nChanAmpA++;
  }
  inline void fillAmpSideC(const ChannelData_t &channelData, int idxPM)
  {
    mVecAmpC_PM[idxPM]+=data_helper::amp(channelData);
    nChanAmpC++;
  }

  inline void fillTimeSideA(const ChannelData_t &channelData)
  {
    timeA += data_helper::time(channelData);
    nChanTimeA++;
  }
  inline void fillTimeSideC(const ChannelData_t &channelData)
  {
    timeC += data_helper::time(channelData);
    nChanTimeC++;
  }

  void finalize()
  {
    amplA = std::accumulate(mVecAmpA_PM.begin(), mVecAmpA_PM.end(), 0, [](auto &&sum, auto &&curr) {return sum + (curr>>3);});
    amplC = std::accumulate(mVecAmpC_PM.begin(), mVecAmpC_PM.end(), 0, [](auto &&sum, auto &&curr) {return sum + (curr>>3);});
    amplSum = amplA + amplC;
    nChanSum = nChanA + nChanC;
    if (nChanA > 0) {
      timeA = timeA / nChanA;
      triggersignals |= (1 << Triggers_t::bitA);
    }
    if (nChanC > 0) {
      timeC = timeC / nChanC;
      triggersignals |= (1 << Triggers_t::bitC);
    }
  }
};
template <typename DigitType, typename ChannelDataType>
class TrgValidation
{
 public:
  using ChannelData_t = ChannelDataType;
  using Digit_t = DigitType;
  typedef decltype(std::declval<Digit_t>().mTriggers) Triggers_t;
  using DataTCM_t = DataTCM<Digit_t, ChannelData_t>;
  void configure(const quality_control::core::CustomParameters& params)
  {
    mGoodPMbits_ChID = o2::quality_control_modules::common::getFromConfig<int>(params, "goodPMbits_ChID", -192);

    mTrgModeThresholdVar = o2::quality_control_modules::common::getFromConfig<std::string>(params, "trgModeThresholdVar", "Ampl");
    mTrgModeSide = o2::quality_control_modules::common::getFromConfig<std::string>(params, "trgModeSide", "A+C");

    try:
      mFunctorTrgCalc = mMapTrgCalcFunctors[std::make_pair(mTrgModeThresholdVar, mTrgModeSide)];
    
    mTrgOrGate = o2::quality_control_modules::common::getFromConfig<int>(params, "trgOrGate", 153);
    mTrgThresholdTimeLow = o2::quality_control_modules::common::getFromConfig<int>(params, "trgThresholdTimeLow", -100);
    mTrgThresholdTimeHigh = o2::quality_control_modules::common::getFromConfig<int>(params, "trgThresholdTimeHigh", 100);

    mTrgChargeLevelLow = o2::quality_control_modules::common::getFromConfig<int>(params, "trgChargeLevelLow", 0);
    mTrgChargeLevelHigh = o2::quality_control_modules::common::getFromConfig<int>(params, "trgChargeLevelHigh", 4095);
    mTrgThresholdCenA = o2::quality_control_modules::common::getFromConfig<int>(params, "trgThresholdCenA", 20);
    mTrgThresholdCenC = o2::quality_control_modules::common::getFromConfig<int>(params, "trgThresholdCenC", 20);
    mTrgThresholdSCenA = o2::quality_control_modules::common::getFromConfig<int>(params, "trgThresholdSCenA", 10);
    mTrgThresholdSCenC = o2::quality_control_modules::common::getFromConfig<int>(params, "trgThresholdSCenC", 10);
  }
  void processChannelData(DataTCM &dataTCM, ChannelData_t &&) {
    
  }
  DataTCM_t mDataTCM{}; // TCM emulator object
  std::string mTrgModeThresholdVar{};
  std::string mMapTrgCalcFunctors{};
  // time window for vertex trigger
  int mTrgThresholdTimeLow;
  int mTrgThresholdTimeHigh;
  int mTrgOrGate;
  // parameters for (Semi)Central triggers
  // same parameters re-used for both Ampl and Nchannels thresholds
  int mTrgThresholdCenA;
  int mTrgThresholdCenC;
  int mTrgThresholdSCenA;
  int mTrgThresholdSCenC;
  int mTrgChargeLevelLow;
  int mTrgChargeLevelHigh;
  int mTrgOrGate;

  enum ETriggerValidation { // first bit - HW, second - SW
    kBothOff = 0b00,
    kOnlyHW = 0b01,
    kOnlySW = 0b10,
    kBothOn = 0b11
  };
  const std::map<unsigned int, std::string> mMapTrgValidation = {
    { ETriggerValidation::kBothOff, "Both off" },
    { ETriggerValidation::kOnlyHW, "Only HW" },
    { ETriggerValidation::kOnlySW, "Only SW" },
    { ETriggerValidation::kBothOn, "Both on" }
  };
  // To get trigger validation status
  inline static ETriggerValidation getTrgValidationStatus(uint8_t hwTrg, uint8_t swTrg, uint8_t trgBitPos)
  {
    uint8_t status = (((hwTrg >> trgBitPos) & 1) | (((swTrg << 1) >> trgBitPos) & 0b10));
    return static_cast<ETriggerValidation>(status);
  };
  static const inline std::map<std::pair<std::string, std::string>, std::function<void(DataTCM_t&)>> mMapTrgCalcFunctors = {
    { { "Ampl", "A+C" }, [this](DataTCM_t& tcm) {
       const uint8_t cent = ((tcm.amplSum >= mTrgThresholdCenA) << Triggers_t::bitCen);
       const uint8_t trg = ((tcm.amplSum >= mTrgThresholdSCenA && cent == 0) << Triggers_t::bitSCen) | cent;
       tcm.triggersignals |= trg;
     } },
    { { "Ampl", "A&C" }, [this](DataTCM_t& tcm) {
       const uint8_t cent = ((tcm.amplA >= mTrgThresholdCenA && tcm.amplC >= mTrgThresholdCenC) << Triggers_t::bitCen);
       const uint8_t trg = (((tcm.amplA >= mTrgThresholdSCenA && tcm.amplC >= mTrgThresholdSCenC) && cent == 0) << Triggers_t::bitSCen) | cent;
       tcm.triggersignals |= trg;
     } },
    { { "Ampl", "A" }, [this](DataTCM_t& tcm) {
       const uint8_t cent = ((tcm.amplA >= mTrgThresholdCenA) << Triggers_t::bitCen);
       const uint8_t trg = ((tcm.amplA >= mTrgThresholdSCenA && cent == 0) << Triggers_t::bitSCen) | cent;
       tcm.triggersignals |= trg;
     } },
    { { "Ampl", "C" }, [this](DataTCM_t& tcm) {
       const uint8_t cent = ((tcm.amplC >= mTrgThresholdCenC) << Triggers_t::bitCen);
       const uint8_t trg = ((tcm.amplC >= mTrgThresholdSCenC && cent == 0) << Triggers_t::bitSCen) | cent;
       tcm.triggersignals |= trg;
     } },
    { { "Nchannels", "A+C" }, [this](DataTCM_t& tcm) {
       const uint8_t cent = ((tcm.nChanSum >= mTrgThresholdCenA) << Triggers_t::bitCen);
       const uint8_t trg = ((tcm.nChanSum >= mTrgThresholdSCenA && cent == 0) << Triggers_t::bitSCen) | cent;
       tcm.triggersignals |= trg;
     } },
    { { "Nchannels", "A&C" }, [this](DataTCM_t& tcm) {
       const uint8_t cent = ((tcm.nChanA >= mTrgThresholdCenA && tcm.nChanC >= mTrgThresholdCenC) << Triggers_t::bitCen);
       const uint8_t trg = (((tcm.nChanA >= mTrgThresholdSCenA && tcm.nChanC >= mTrgThresholdSCenC) && cent == 0) << Triggers_t::bitSCen) | cent;
       tcm.triggersignals |= trg;
     } },
    { { "Nchannels", "A" }, [this](DataTCM_t& tcm) {
       const uint8_t cent = ((tcm.nChanA >= mTrgThresholdCenA) << Triggers_t::bitCen);
       const uint8_t trg = ((tcm.nChanA >= mTrgThresholdSCenA && cent == 0) << Triggers_t::bitSCen) | cent;
       tcm.triggersignals |= trg;
     } },
    { { "Nchannels", "C" }, [this](DataTCM_t& tcm) {
       const uint8_t cent = ((tcm.nChanC >= mTrgThresholdCenC) << Triggers_t::bitCen);
       const uint8_t trg = ((tcm.nChanC >= mTrgThresholdSCenC && cent == 0) << Triggers_t::bitSCen) | cent;
       tcm.triggersignals |= trg;
     } }
  };
  std::function<void(DataTCM_t&)> mFunctorAmpTrgCalc;
};

} // namespace o2::quality_control_modules::fit
#endif
