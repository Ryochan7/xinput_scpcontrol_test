#ifndef AXISCURVE_H
#define AXISCURVE_H

class AxisCurve
{
public:
    typedef enum {
        Linear,
        EnhancedPrecision,
        Quadratic,
        Cubic,
        Power,
        Disabled,
    } Type;

};

#endif // AXISCURVE_H
