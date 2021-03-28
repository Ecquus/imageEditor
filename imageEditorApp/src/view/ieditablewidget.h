#pragma once

class IEditableWidget
{
public:
    virtual ~IEditableWidget() { }

    virtual void clearFocus() = 0;
    virtual void resetValues() = 0;
};
