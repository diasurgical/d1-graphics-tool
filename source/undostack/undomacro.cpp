#include "undomacro.h"

#include <utility>

UserData::UserData(QString labelText, QString cancelButtonText, std::pair<int, int> &&minMax)
    : m_labelText(std::move(labelText))
    , m_cancelButtonText(std::move(cancelButtonText))
    , m_minMax(minMax)
{
}

UserData::UserData(QString labelText, QString cancelButtonText)
    : m_labelText(std::move(labelText))
    , m_cancelButtonText(std::move(cancelButtonText))
    , m_minMax({ 0, 0 })
{
}

UndoMacroFactory::UndoMacroFactory(UserData &&userData)
    : m_userData(std::make_unique<UserData>(userData.labelText(), userData.cancelButtonText(), std::make_pair(userData.min(), userData.max())))
{
}

void UndoMacroFactory::setUserData(const UserData &&userData)
{
    m_userData = std::make_unique<UserData>(userData.labelText(), userData.cancelButtonText(), std::make_pair(userData.min(), userData.max()));
}

void UndoMacroFactory::add(std::unique_ptr<Command> cmd)
{
    m_commands.push_back(std::move(cmd));
}

UndoMacro::UndoMacro(std::unique_ptr<UserData> userData, std::pair<int, int> rangeIdxs)
    : m_userData(std::move(userData))
    , m_rangeIdxs(rangeIdxs)
{
}

UndoMacro::UndoMacro(UndoMacro &&undoMacro) noexcept
    : m_userData(std::move(undoMacro.m_userData))
    , m_rangeIdxs(undoMacro.m_rangeIdxs)
{
}

UndoMacro &UndoMacro::operator=(UndoMacro &&undoMacro) noexcept
{
    m_userData = std::move(undoMacro.m_userData);
    m_rangeIdxs = undoMacro.m_rangeIdxs;
    return *this;
}

void UndoMacro::setLastIndex(int index)
{
    m_rangeIdxs.second = index;
    /* TODO: later on, if/when we will implement more types of macros, we could use some update method of UserData
     * instead of using setters directly
     */
    m_userData->setMax((m_rangeIdxs.second - m_rangeIdxs.first) + 1);
}
