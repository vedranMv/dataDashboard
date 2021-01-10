#ifndef MATHCHANNEL_H
#define MATHCHANNEL_H

#include <QString>
#include <vector>
#include <tuple>
#include <QVector>

/**
 * @brief The SignalSource enum
 *      Describing signal origin
 */
enum SignalSource {
    AllChannels,
    SerialSignal,
    MathSignal,
    NetworkSignal
};

/**
 * @brief The MathOperation enum
 *      Math operation ID (used in math componenets)
 */
enum MathOperation {
    Add_Signal,
    Subtract_Signal,
    Multiply,
    Add_Abs,
    Subtract_Abs,
    Multiply_Abs
};

/**
 * @brief The MathOperationText lookup table
 *      Links \ref enum MathOperation ID to a string
 */
const QVector<QString> MathOperationText {
    "+",
    "-",
    "*",
    "+ ABS",
    " - ABS",
    "* ABS"
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
    /**
     * @brief Add new math component to this math channel
     * @param operation id of math operation
     * @param inputChannel input channel to use in the component
     */
    void AddComponent(MathOperation operation, uint8_t inputChannel)
    {
        _component.push_back(std::tuple<MathOperation,uint8_t>(operation,inputChannel));
    }
    /**
     * @brief Reset the component
     */
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
