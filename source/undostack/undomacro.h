#pragma once

#include "command.h"
#include <QString>
#include <memory>
#include <string_view>
#include <vector>

class UserData {
private:
    QString m_labelText;
    QString m_cancelButtonText;
    std::pair<int, int> m_minMax;

public:
    UserData(QString labelText, QString cancelButtonText, std::pair<int, int> &&minMax);
    UserData(QString labelText, QString cancelButtonText);
    ~UserData() = default;

    [[nodiscard]] int min() const
    {
        return m_minMax.first;
    }
    [[nodiscard]] int max() const
    {
        return m_minMax.second;
    }
    [[nodiscard]] const QString &labelText() const
    {
        return m_labelText;
    }
    [[nodiscard]] const QString &cancelButtonText() const
    {
        return m_cancelButtonText;
    }
    void setMin(int min)
    {
        m_minMax.first = min;
    }
    void setMax(int max)
    {
        m_minMax.second = max;
    }
};

class UndoMacroFactory {
private:
    std::unique_ptr<UserData> m_userData;
    std::vector<std::unique_ptr<Command>> m_commands;

public:
    UndoMacroFactory(UserData &&userData);
    UndoMacroFactory() = default;
    ~UndoMacroFactory() = default;

    void setUserData(const UserData &&userData);

    void add(std::unique_ptr<Command> cmd);
    [[nodiscard]] std::vector<std::unique_ptr<Command>> &cmds()
    {
        return m_commands;
    };
    [[nodiscard]] std::unique_ptr<UserData> &userdata()
    {
        return m_userData;
    };
};

class UndoMacro {
private:
    std::unique_ptr<UserData> m_userData;
    std::pair<int, int> m_rangeIdxs;

public:
    UndoMacro(std::unique_ptr<UserData> userData, std::pair<int, int> rangeIdxs);
    UndoMacro(UndoMacro &&undoMacro) noexcept;
    UndoMacro &operator=(UndoMacro &&undoMacro) noexcept;
    ~UndoMacro() = default;

    [[nodiscard]] std::unique_ptr<UserData> &userdata()
    {
        return m_userData;
    };
    [[nodiscard]] int beginIndex() const
    {
        return m_rangeIdxs.first;
    }
    [[nodiscard]] int lastIndex() const
    {
        return m_rangeIdxs.second;
    }
    void setLastIndex(int index);
};
