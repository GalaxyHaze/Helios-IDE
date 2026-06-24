/****************************************************************************
** Meta object code from reading C++ file 'LspClient.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../editor/LspClient.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LspClient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN9LspClientE_t {};
} // unnamed namespace

template <> constexpr inline auto LspClient::qt_create_metaobjectdata<qt_meta_tag_ZN9LspClientE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "LspClient",
        "initialized",
        "",
        "serverError",
        "message",
        "diagnosticsReceived",
        "uri",
        "QList<LspDiagnostic>",
        "diagnostics",
        "completionResults",
        "QList<LspCompletionItem>",
        "items",
        "hoverResult",
        "LspHoverInfo",
        "info",
        "definitionResult",
        "LspLocation",
        "location",
        "signatureHelpResult",
        "LspSignatureHelp",
        "help",
        "semanticTokensResult",
        "QJsonArray",
        "tokens",
        "formattingResult",
        "QList<std::pair<LspRange,QString>>",
        "edits",
        "documentSymbolsResult",
        "symbols",
        "logMessage",
        "onReadyRead",
        "onProcessError",
        "QProcess::ProcessError",
        "error",
        "onProcessFinished",
        "exitCode",
        "QProcess::ExitStatus",
        "status"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'initialized'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'serverError'
        QtMocHelpers::SignalData<void(const QString &)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 4 },
        }}),
        // Signal 'diagnosticsReceived'
        QtMocHelpers::SignalData<void(const QString &, const QList<LspDiagnostic> &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { 0x80000000 | 7, 8 },
        }}),
        // Signal 'completionResults'
        QtMocHelpers::SignalData<void(const QList<LspCompletionItem> &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 },
        }}),
        // Signal 'hoverResult'
        QtMocHelpers::SignalData<void(const LspHoverInfo &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 13, 14 },
        }}),
        // Signal 'definitionResult'
        QtMocHelpers::SignalData<void(const LspLocation &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 17 },
        }}),
        // Signal 'signatureHelpResult'
        QtMocHelpers::SignalData<void(const LspSignatureHelp &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 19, 20 },
        }}),
        // Signal 'semanticTokensResult'
        QtMocHelpers::SignalData<void(const QJsonArray &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 22, 23 },
        }}),
        // Signal 'formattingResult'
        QtMocHelpers::SignalData<void(const QList<QPair<LspRange,QString>> &)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 25, 26 },
        }}),
        // Signal 'documentSymbolsResult'
        QtMocHelpers::SignalData<void(const QJsonArray &)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 22, 28 },
        }}),
        // Signal 'logMessage'
        QtMocHelpers::SignalData<void(const QString &)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 4 },
        }}),
        // Slot 'onReadyRead'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onProcessError'
        QtMocHelpers::SlotData<void(QProcess::ProcessError)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 32, 33 },
        }}),
        // Slot 'onProcessFinished'
        QtMocHelpers::SlotData<void(int, QProcess::ExitStatus)>(34, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 35 }, { 0x80000000 | 36, 37 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<LspClient, qt_meta_tag_ZN9LspClientE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject LspClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9LspClientE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9LspClientE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9LspClientE_t>.metaTypes,
    nullptr
} };

void LspClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LspClient *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->initialized(); break;
        case 1: _t->serverError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->diagnosticsReceived((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<LspDiagnostic>>>(_a[2]))); break;
        case 3: _t->completionResults((*reinterpret_cast<std::add_pointer_t<QList<LspCompletionItem>>>(_a[1]))); break;
        case 4: _t->hoverResult((*reinterpret_cast<std::add_pointer_t<LspHoverInfo>>(_a[1]))); break;
        case 5: _t->definitionResult((*reinterpret_cast<std::add_pointer_t<LspLocation>>(_a[1]))); break;
        case 6: _t->signatureHelpResult((*reinterpret_cast<std::add_pointer_t<LspSignatureHelp>>(_a[1]))); break;
        case 7: _t->semanticTokensResult((*reinterpret_cast<std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 8: _t->formattingResult((*reinterpret_cast<std::add_pointer_t<QList<std::pair<LspRange,QString>>>>(_a[1]))); break;
        case 9: _t->documentSymbolsResult((*reinterpret_cast<std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 10: _t->logMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->onReadyRead(); break;
        case 12: _t->onProcessError((*reinterpret_cast<std::add_pointer_t<QProcess::ProcessError>>(_a[1]))); break;
        case 13: _t->onProcessFinished((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QProcess::ExitStatus>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LspClient::*)()>(_a, &LspClient::initialized, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (LspClient::*)(const QString & )>(_a, &LspClient::serverError, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (LspClient::*)(const QString & , const QList<LspDiagnostic> & )>(_a, &LspClient::diagnosticsReceived, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (LspClient::*)(const QList<LspCompletionItem> & )>(_a, &LspClient::completionResults, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (LspClient::*)(const LspHoverInfo & )>(_a, &LspClient::hoverResult, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (LspClient::*)(const LspLocation & )>(_a, &LspClient::definitionResult, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (LspClient::*)(const LspSignatureHelp & )>(_a, &LspClient::signatureHelpResult, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (LspClient::*)(const QJsonArray & )>(_a, &LspClient::semanticTokensResult, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (LspClient::*)(const QList<QPair<LspRange,QString>> & )>(_a, &LspClient::formattingResult, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (LspClient::*)(const QJsonArray & )>(_a, &LspClient::documentSymbolsResult, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (LspClient::*)(const QString & )>(_a, &LspClient::logMessage, 10))
            return;
    }
}

const QMetaObject *LspClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LspClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9LspClientE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LspClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void LspClient::initialized()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LspClient::serverError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void LspClient::diagnosticsReceived(const QString & _t1, const QList<LspDiagnostic> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void LspClient::completionResults(const QList<LspCompletionItem> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void LspClient::hoverResult(const LspHoverInfo & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void LspClient::definitionResult(const LspLocation & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void LspClient::signatureHelpResult(const LspSignatureHelp & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void LspClient::semanticTokensResult(const QJsonArray & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void LspClient::formattingResult(const QList<QPair<LspRange,QString>> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void LspClient::documentSymbolsResult(const QJsonArray & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void LspClient::logMessage(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}
QT_WARNING_POP
