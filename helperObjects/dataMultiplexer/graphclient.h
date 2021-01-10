#ifndef GRAPHCLIENT_H
#define GRAPHCLIENT_H

#include <plotWindows/plotWindows.h>

enum GraphType {
    Orientation3D,
    Scatter,
    Line
};


class GraphClient
{
    friend class DataMultiplexer;
public:
    GraphClient(QString name, uint8_t nInChannels, OrientationWindow* reciver) \
        :_name(name), _inChannels(nInChannels), _reciver3D(reciver)
    { _type = GraphType::Orientation3D; }
    GraphClient(QString name, uint8_t nInChannels, ScatterWindow* reciver) \
        :_name(name), _inChannels(nInChannels), _receiverScatter(reciver)
    { _type = GraphType::Scatter; }
    GraphClient(QString name, uint8_t nInChannels, LinePlot* reciver) \
        :_name(name), _inChannels(nInChannels), _receiverLine(reciver)
    { _type = GraphType::Line; }

    void SetInputChannels(uint8_t n, uint8_t *chList)
    {
        _inputChannelMap.clear();

        for (uint8_t i = 0; i < n; i++)
            _inputChannelMap.push_back(chList[i]);
    }

    void SendData(uint8_t n, double* data)
    {
        switch (_type)
        {
        case GraphType::Orientation3D:
            _reciver3D->ReceiveData(data, n);
            break;
        case GraphType::Scatter:
            _receiverScatter->ReceiveData(data, n);
            break;
        case GraphType::Line:
            _receiverLine->ReceiveData(data, n);
            break;
        default:
            break;
        }

    }

    const QString& GetName()
    {
        return _name;
    }

    OrientationWindow* Receiver(OrientationWindow* dummy=0)
    {
        assert(dummy==dummy);
        return _reciver3D;
    }

    ScatterWindow* Receiver(ScatterWindow* dummy=0)
    {
        assert(dummy==dummy);
        return _receiverScatter;
    }

    LinePlot* Receiver(LinePlot* dummy=0)
    {
        assert(dummy==dummy);
        return _receiverLine;
    }


private:
    GraphType _type;
    QString _name;
    uint8_t _inChannels;
    OrientationWindow* _reciver3D;
    ScatterWindow* _receiverScatter;
    LinePlot    *_receiverLine;
    std::vector<uint8_t>_inputChannelMap;
};
#endif // GRAPHCLIENT_H
