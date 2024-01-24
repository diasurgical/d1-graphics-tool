#pragma once

class Command {
public:
    virtual void undo() = 0;
    virtual void redo() = 0;

    void setObsolete(bool isObsolete);
    bool isObsolete() const;
    void setMacroID(unsigned int macroID);
    unsigned int macroID() const;

    virtual ~Command() = default;

private:
    unsigned int m_macroID { 0 };
    bool m_isObsolete = false;
};
