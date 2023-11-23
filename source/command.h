#pragma once

class Command {
public:
    virtual void undo() = 0;
    virtual void redo() = 0;

    void setObsolete(bool isObsolete);
    bool isObsolete() const;

    virtual ~Command() = default;

private:
    bool m_isObsolete = false;
};
