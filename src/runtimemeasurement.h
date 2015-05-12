/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */

#ifndef SRC_RUNTIMEMEASUREMENT_H_
#define SRC_RUNTIMEMEASUREMENT_H_

#define BOOST_CHRONO_VERSION 2
#include <boost/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <memory>  // Remove when GCC 5.0 is widespread
                   // (as it implements move for file streams)
#include "./circuitcocircuit.h"

using ogdf::List;

class RuntimeMeasurement {
    typedef boost::chrono::process_user_cpu_clock clock_type;
    typedef boost::chrono::time_point<clock_type> time_point;

    std::unique_ptr<ofstream> fout;

    time_point now() {
        return boost::chrono::process_user_cpu_clock::now();
    }

 public:
    typedef struct {
        time_point time;
        int nBonds;
    } point;

    RuntimeMeasurement() {
        // Replace with move(ofstream(..)) (see #include <memory> above)
        fout = std::unique_ptr<ofstream>(new ofstream("mincuts_rtm.log"));
        *fout << boost::chrono::symbol_format;
    }

    /**
     * @brief Marks a point in the progress of the algorithm
     * @param j     the current stage of the algorithm
     * @param bonds
     * @return
     */
    RuntimeMeasurement::point mark(int nBondsOutput) {
        RuntimeMeasurement::point p = {
            now(),  // time
            nBondsOutput
        };

        return p;
    }

    /**
     * @param j     the current stage of the algorithm
     * @param bonds
     * @param start created with the `mark` method
     */
    void log(int j, int nBondsOutput, const point &start) {
        point end = mark(nBondsOutput);

        *fout << j << "\t" \
              << boost::chrono::duration_cast<boost::chrono::milliseconds>(end.time - start.time).count() << "\t" \
              << end.nBonds - start.nBonds \
              << "\n";
    }
};

#endif  // SRC_RUNTIMEMEASUREMENT_H_
