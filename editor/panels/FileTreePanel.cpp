#include "FileTreePanel.h"
#include "../widgets/ProjectTreeModel.h"
#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"
#include <QTreeView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QClipboard>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

FileTreePanel::FileTreePanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_model = new ProjectTreeModel(this);

    m_treeView = new QTreeView;
    m_treeView->setModel(m_model);
    m_treeView->setHeaderHidden(true);
    m_treeView->setAnimated(true);
    m_treeView->setUniformRowHeights(true);
    m_treeView->setIndentation(12);
    m_treeView->setRootIsDecorated(true);
    m_treeView->setExpandsOnDoubleClick(true);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    layout->addWidget(m_treeView);

    connect(m_treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex &idx) {
        if (idx.isValid()) {
            auto *node = static_cast<ProjectTreeNode*>(idx.internalPointer());
            if (node && node->isLoadMoreNode) {
                m_model->loadMore(idx);
            } else {
                QString path = m_model->filePath(idx);
                if (!path.isEmpty() && QFileInfo(path).isFile()) {
                    emit fileActivated(path);
                }
            }
        }
    });

    connect(m_treeView, &QTreeView::customContextMenuRequested, this, &FileTreePanel::showContextMenu);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FileTreePanel::updateThemeAndLanguage);
    connect(&TranslationManager::instance(), &TranslationManager::localeChanged, this, &FileTreePanel::updateThemeAndLanguage);

    updateThemeAndLanguage();
}

void FileTreePanel::setRootPath(const QString &path)
{
    m_model->setRootPath(path);
    if (!path.isEmpty()) {
        QModelIndex rootIdx = m_model->index(0, 0);
        m_treeView->expand(rootIdx);
    }
}

QString FileTreePanel::rootPath() const
{
    return m_model->rootPath();
}

void FileTreePanel::showContextMenu(const QPoint &pos)
{
    QModelIndex idx = m_treeView->indexAt(pos);
    QString targetPath = idx.isValid() ? m_model->filePath(idx) : m_model->rootPath();
    if (targetPath.isEmpty()) {
        targetPath = m_model->rootPath();
    }

    QMenu *menu = new QMenu(this);
    auto &tm = ThemeManager::instance();
    auto &tr = TranslationManager::instance();

    QAction *newFileAct = menu->addAction(tr.translate("menu.new_file"));
    QAction *newDirAct = menu->addAction(tr.translate("menu.new_dir"));
    menu->addSeparator();
    QAction *openAct = menu->addAction(tr.translate("menu.open"));
    QAction *openTabAct = menu->addAction(tr.translate("menu.open_tab"));
    QAction *revealAct = menu->addAction(tr.translate("menu.reveal"));
    menu->addSeparator();
    QAction *copyAbsAct = menu->addAction(tr.translate("menu.copy_abs"));
    QAction *copyRelAct = menu->addAction(tr.translate("menu.copy_rel"));
    menu->addSeparator();
    QAction *renameAct = menu->addAction(tr.translate("menu.rename"));
    QAction *deleteAct = menu->addAction(tr.translate("menu.delete"));
    menu->addSeparator();
    QAction *refreshAct = menu->addAction(tr.translate("menu.refresh"));
    QAction *collapseAllAct = menu->addAction(tr.translate("menu.collapse_all"));
    QAction *setRootAct = menu->addAction(tr.translate("menu.set_root"));

    // Enable/disable actions depending on context
    bool hasValidNode = idx.isValid();
    bool isFolder = hasValidNode ? m_model->isDir(idx) : true;
    openAct->setEnabled(hasValidNode && !isFolder);
    openTabAct->setEnabled(hasValidNode && !isFolder);
    renameAct->setEnabled(hasValidNode);
    deleteAct->setEnabled(hasValidNode);
    setRootAct->setEnabled(hasValidNode && isFolder);

    // Apply menu stylesheet
    menu->setStyleSheet(
        QString(
            "QMenu { background: %1; color: %2; border: 1px solid %3; }"
            "QMenu::item:selected { background: %4; color: %5; }"
            "QMenu::item:disabled { color: #6c7086; }"
        ).arg(tm.palette().color(QPalette::Base).name(),
              tm.palette().color(QPalette::Text).name(),
              tm.customColor("sidebarBorder", QColor("#363a4f")).name(),
              tm.palette().color(QPalette::Highlight).name(),
              tm.palette().color(QPalette::HighlightedText).name())
    );

    QAction *selectedAct = menu->exec(m_treeView->mapToGlobal(pos));
    if (!selectedAct) {
        delete menu;
        return;
    }

    // Determine target directory for file/dir creation
    QString baseDir = targetPath;
    if (hasValidNode && !isFolder) {
        baseDir = QFileInfo(targetPath).absolutePath();
    }

    if (selectedAct == newFileAct) {
        bool ok;
        QString name = QInputDialog::getText(this, tr.translate("dialog.new_file_title"),
                                             tr.translate("dialog.new_file_prompt"),
                                             QLineEdit::Normal, "", &ok);
        if (ok && !name.trimmed().isEmpty()) {
            QString path = QDir(baseDir).filePath(name);
            QFile file(path);
            if (file.open(QIODevice::WriteOnly)) {
                file.close();
                m_model->refresh(idx.isValid() ? idx.parent() : QModelIndex());
            } else {
                QMessageBox::warning(this, "Error", "Could not create file: " + path);
            }
        }
    } else if (selectedAct == newDirAct) {
        bool ok;
        QString name = QInputDialog::getText(this, tr.translate("dialog.new_dir_title"),
                                             tr.translate("dialog.new_dir_prompt"),
                                             QLineEdit::Normal, "", &ok);
        if (ok && !name.trimmed().isEmpty()) {
            QString path = QDir(baseDir).filePath(name);
            if (QDir().mkdir(path)) {
                m_model->refresh(idx.isValid() ? idx.parent() : QModelIndex());
            } else {
                QMessageBox::warning(this, "Error", "Could not create directory: " + path);
            }
        }
    } else if (selectedAct == openAct) {
        emit fileActivated(targetPath);
    } else if (selectedAct == openTabAct) {
        emit fileActivatedInNewTab(targetPath);
    } else if (selectedAct == revealAct) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(baseDir));
    } else if (selectedAct == copyAbsAct) {
        QApplication::clipboard()->setText(QFileInfo(targetPath).absoluteFilePath());
    } else if (selectedAct == copyRelAct) {
        QApplication::clipboard()->setText(QDir(m_model->rootPath()).relativeFilePath(targetPath));
    } else if (selectedAct == renameAct) {
        bool ok;
        QFileInfo info(targetPath);
        QString name = QInputDialog::getText(this, tr.translate("dialog.rename_title"),
                                             tr.translate("dialog.rename_prompt"),
                                             QLineEdit::Normal, info.fileName(), &ok);
        if (ok && !name.trimmed().isEmpty() && name != info.fileName()) {
            QString newPath = info.absoluteDir().filePath(name);
            if (QFile::rename(targetPath, newPath)) {
                m_model->refresh(idx.parent());
            } else {
                QMessageBox::warning(this, "Error", "Could not rename to: " + name);
            }
        }
    } else if (selectedAct == deleteAct) {
        auto result = QMessageBox::question(this, tr.translate("dialog.confirm_delete_title"),
                                            tr.translate("dialog.confirm_delete_msg").arg(QFileInfo(targetPath).fileName()),
                                            QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes) {
            bool ok = false;
            if (isFolder) {
                ok = QDir(targetPath).removeRecursively();
            } else {
                ok = QFile::remove(targetPath);
            }
            if (ok) {
                m_model->refresh(idx.parent());
            } else {
                QMessageBox::warning(this, "Error", "Could not delete: " + targetPath);
            }
        }
    } else if (selectedAct == refreshAct) {
        m_model->refresh(idx);
    } else if (selectedAct == collapseAllAct) {
        m_treeView->collapseAll();
    } else if (selectedAct == setRootAct) {
        emit projectRootChanged(targetPath);
    }

    delete menu;
}

void FileTreePanel::updateThemeAndLanguage()
{
    auto &tm = ThemeManager::instance();

    QPalette pal = tm.palette();
    setPalette(pal);

    QColor base = pal.color(QPalette::Base);
    QColor text = pal.color(QPalette::Text);
    QColor windowText = pal.color(QPalette::WindowText);

    QString textHex = text.name();
    QString bgHex = base.name();
    QString windowTextHex = windowText.name();
    QString itemHoverHex = tm.customColor("treeHover", QColor("#363a4f")).name();

    m_treeView->setStyleSheet(
        QString(
            "QTreeView { background: %1; color: %2; border: none; font-size: 13px; }"
            "QTreeView::item { padding: 4px 6px; color: %2; }"
            "QTreeView::item:hover { background: %3; }"
            "QTreeView::item:selected { background: %3; color: %4; }"
            "QTreeView::branch { background: %1; }"
        ).arg(bgHex, textHex, itemHoverHex, windowTextHex)
    );
}
