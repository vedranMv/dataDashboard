#ifndef MATHCHANNEL_H
#define MATHCHANNEL_H

#include <QString>
#include <vector>
#include <tuple>


enum SignalSource {
    AllChannels,
    SerialSignal,
    MathSignal,
    NetworkSignal
};

enum MathOperation {
    Add_Signal,
    Subtract_Signal,
    Multiply,
    Add_Abs,
    Subtract_Abs,
    Multiply_Abs
};



/**
 * @brief The MathChannel class
 *      Helper class for storing and manipulating math channels
 *      and their components
 */
class MathChannel
{
public:
    MathChannel(): Enabled(false) {};
    void SetLabel(QString label)
    {
         _label = label;
    }
    QString GetLabel()
    {
         return _label;
    }

    void AddComponent(MathOperation operation, uint8_t serialChannel)
    {
        _component.push_back(std::tuple<MathOperation,uint8_t>(operation,serialChannel));
    }

    void Clear()
    {
        Enabled = false;
        _label = QString("");
        _component.clear();
    }

    bool Enabled;

//private:
    QString _label;
    std::vector< std::tuple<MathOperation,uint8_t> >_component;
};


#endif // MATHCHANNEL_H
