
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file simpleswap.cpp
    \brief Simple fixed-rate vs Libor swap

    $Id$
*/

#include "ql/Instruments/simpleswap.hpp"
#include "ql/CashFlows/cashflowvectors.hpp"

namespace QuantLib {

    using CashFlows::FixedRateCoupon;
    using CashFlows::FixedRateCouponVector;
    using CashFlows::ParCouponVector;
    using Indexes::Xibor;
    
    namespace Instruments {

        SimpleSwap::SimpleSwap(bool payFixedRate,
          const Date& startDate, int n, TimeUnit units,
          const Handle<Calendar>& calendar, 
          RollingConvention rollingConvention, 
          const std::vector<double>& nominals, 
          int fixedFrequency, 
          const std::vector<Rate>& couponRates, 
          bool fixedIsAdjusted, 
          const Handle<DayCounter>& fixedDayCount, 
          int floatingFrequency, 
          const Xibor& index, 
          const std::vector<Spread>& spreads, 
          const Handle<DayCounter>& floatingDayCount, 
          const RelinkableHandle<TermStructure>& termStructure, 
          const std::string& isinCode, const std::string& description)
        : Swap(std::vector<Handle<CashFlow> >(), 
               std::vector<Handle<CashFlow> >(),
               termStructure, isinCode, description), 
          payFixedRate_(payFixedRate) {
            
            maturity_ = calendar->advance(startDate,n,units,rollingConvention);
            if (payFixedRate_) {
                firstLeg_ = FixedRateCouponVector(nominals, 
                    couponRates, startDate, maturity_, 
                    fixedFrequency, calendar, rollingConvention, 
                    fixedIsAdjusted, fixedDayCount);
                secondLeg_ = ParCouponVector(nominals, 
                    index, spreads, startDate, maturity_, floatingFrequency, 
                    calendar, rollingConvention, floatingDayCount,
                    termStructure);
            } else {
                firstLeg_ = ParCouponVector(nominals, 
                    index, spreads, startDate, maturity_, floatingFrequency, 
                    calendar, rollingConvention, floatingDayCount,
                    termStructure);
                secondLeg_ = FixedRateCouponVector(nominals, 
                    couponRates, startDate, maturity_, 
                    fixedFrequency, calendar, rollingConvention, 
                    fixedIsAdjusted, fixedDayCount);
            }
        }

        void SimpleSwap::performCalculations() const {
            Swap::performCalculations();
            BPS_ = 0.0;
            std::vector<Handle<CashFlow> >::const_iterator begin, end;
            if (payFixedRate_) {
                begin = firstLeg_.begin();
                end   = firstLeg_.end();
            } else {
                begin = secondLeg_.begin();
                end   = secondLeg_.end();
            }
            for (; begin != end; ++begin) {
                // the following should be safe as long as nobody 
                // messed with the coupons
                #if QL_ALLOW_TEMPLATE_METHOD_CALLS
                const FixedRateCoupon* coupon = 
                    begin->downcast<FixedRateCoupon>();
                #else
                const FixedRateCoupon* coupon = 
                    dynamic_cast<const FixedRateCoupon*>(begin->pointer());
                #endif
                // however, we will check that it succeeded
                if (coupon != 0) {
                    BPS_ += coupon->accrualPeriod() * 
                            coupon->nominal() *
                            termStructure_->discount(coupon->date());
                }
            }
            if (payFixedRate_)
                BPS_ = -BPS_;
        }

    }

}

