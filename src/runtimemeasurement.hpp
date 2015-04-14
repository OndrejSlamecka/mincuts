#ifndef RUNTIMEMEASUREMENT_HPP
#define RUNTIMEMEASUREMENT_HPP

#define BOOST_CHRONO_VERSION 2
#include <boost/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <memory> // Remove when GCC 5.0 is widespread (as it implements move for file streams)
#include "circuitcocircuit.h"

using ogdf::List;

class RuntimeMeasurement
{
    typedef boost::chrono::time_point<boost::chrono::thread_clock> time_point;

    std::unique_ptr<ofstream> fout;

    time_point now() {
        return boost::chrono::thread_clock::now();
    }

public:
    typedef struct {
        time_point time;
        int nBonds;
    } point;

    RuntimeMeasurement()
    {
        // Replace with move(ofstream(..)) (see #include <memory> above)
        fout = std::unique_ptr<ofstream>(new ofstream("mincuts_rtm.log"));
        *fout << boost::chrono::symbol_format;
    }

    RuntimeMeasurement(const RuntimeMeasurement &RTM) = delete;
    RuntimeMeasurement & operator=(const RuntimeMeasurement &RTM) = delete;

    RuntimeMeasurement &operator=(RuntimeMeasurement &&o)
    {
        fout = move(o.fout);
        return *this;
    }

    RuntimeMeasurement::point mark(const List<bond> &bonds)
    {
        RuntimeMeasurement::point p = {
            now(), // time
            bonds.size()
        };

        return p;
    }

    /**
     * @param j     the current stage of the algorithm
     * @param bonds
     * @param start created with the `mark` method
     */
    void log(int j, const List<bond> &bonds, point start)
    {
        point end = mark(bonds);

        *fout << j << "\t" \
              << end.nBonds - start.nBonds << "\t"  \
              << boost::chrono::duration_cast<boost::chrono::milliseconds>(end.time - start.time).count() \
              << "\n";
    }

    // The rule of 3 doesn't apply here, we're not doing any memory management in the destructor
    virtual ~RuntimeMeasurement() {
        if (fout) { // if this is not being called as part of a move
            (*fout).flush();
        }
    }

};

#endif // RUNTIMEMEASUREMENT_HPP

