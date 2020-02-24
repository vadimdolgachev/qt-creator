#include <coreplugin/messagebox.h>
#include <aggregation/aggregate.h>
#include <utils/fancylineedit.h>
#include <vcsbase/vcscommand.h>
#include <vcsbase/vcsbaseplugin.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
using namespace std::placeholders;
using GitClientMemberFunc = void (GitClient::*)(const QString &);
class GitTopicCache : public Core::IVersionControl::TopicCache
public:
    GitTopicCache(GitClient *client) :
        m_client(client)
    { }

protected:
    QString trackFile(const QString &repository) override
    {
        const QString gitDir = m_client->findGitDirForRepository(repository);
        return gitDir.isEmpty() ? QString() : (gitDir + "/HEAD");
    }

    QString refreshTopic(const QString &repository) override
    {
        return m_client->synchronousTopic(repository);
    }

private:
    GitClient *m_client;
};

class GitLogEditorWidget : public QWidget
{
    Q_DECLARE_TR_FUNCTIONS(Git::Internal::GitLogEditorWidget);
public:
    GitLogEditorWidget()
    {
        auto gitEditor = new GitEditorWidget;
        auto vlayout = new QVBoxLayout;
        auto hlayout = new QHBoxLayout;
        vlayout->setSpacing(0);
        vlayout->setContentsMargins(0, 0, 0, 0);
        auto grepLineEdit = addLineEdit(tr("Filter by message"),
                                        tr("Filter log entries by text in the commit message."));
        auto pickaxeLineEdit = addLineEdit(tr("Filter by content"),
                                           tr("Filter log entries by added or removed string."));
        hlayout->setSpacing(20);
        hlayout->setContentsMargins(0, 0, 0, 0);
        hlayout->addWidget(new QLabel(tr("Filter:")));
        hlayout->addWidget(grepLineEdit);
        hlayout->addWidget(pickaxeLineEdit);
        hlayout->addStretch();
        vlayout->addLayout(hlayout);
        vlayout->addWidget(gitEditor);
        setLayout(vlayout);
        gitEditor->setGrepLineEdit(grepLineEdit);
        gitEditor->setPickaxeLineEdit(pickaxeLineEdit);

        auto textAgg = Aggregation::Aggregate::parentAggregate(gitEditor);
        auto agg = textAgg ? textAgg : new Aggregation::Aggregate;
        agg->add(this);
        agg->add(gitEditor);
        setFocusProxy(gitEditor);
    }

private:
    FancyLineEdit *addLineEdit(const QString &placeholder, const QString &tooltip)
    {
        auto lineEdit = new FancyLineEdit;
        lineEdit->setFiltering(true);
        lineEdit->setToolTip(tooltip);
        lineEdit->setPlaceholderText(placeholder);
        lineEdit->setMaximumWidth(200);
        return lineEdit;
    }
};

const unsigned minimumRequiredVersion = 0x010900;

const VcsBaseSubmitEditorParameters submitParameters {
    Git::Constants::SUBMIT_MIMETYPE,
    Git::Constants::GITSUBMITEDITOR_ID,
    Git::Constants::GITSUBMITEDITOR_DISPLAY_NAME,
    VcsBaseSubmitEditorParameters::DiffRows
};

const VcsBaseEditorParameters commandLogEditorParameters {
    "text/vnd.qtcreator.git.commandlog"
};

const VcsBaseEditorParameters logEditorParameters {
    LogOutput,
    "text/vnd.qtcreator.git.log"
};

const VcsBaseEditorParameters blameEditorParameters {
    AnnotateOutput,
    "text/vnd.qtcreator.git.annotation"
};

const VcsBaseEditorParameters commitTextEditorParameters {
    OtherContent,
    "text/vnd.qtcreator.git.commit"
};

const VcsBaseEditorParameters rebaseEditorParameters {
    OtherContent,
    "text/vnd.qtcreator.git.rebase"
class GitPluginPrivate final : public VcsBase::VcsBasePluginPrivate
{
    Q_OBJECT

public:
    GitPluginPrivate();
    ~GitPluginPrivate() final;

    // IVersionControl
    QString displayName() const final;
    Core::Id id() const final;

    bool isVcsFileOrDirectory(const Utils::FilePath &fileName) const final;

    bool managesDirectory(const QString &directory, QString *topLevel) const final;
    bool managesFile(const QString &workingDirectory, const QString &fileName) const final;
    QStringList unmanagedFiles(const QString &workingDir, const QStringList &filePaths) const final;

    bool isConfigured() const final;
    bool supportsOperation(Operation operation) const final;
    bool vcsOpen(const QString &fileName) final;
    bool vcsAdd(const QString &fileName) final;
    bool vcsDelete(const QString &filename) final;
    bool vcsMove(const QString &from, const QString &to) final;
    bool vcsCreateRepository(const QString &directory) final;

    bool vcsAnnotate(const QString &file, int line) final;
    QString vcsTopic(const QString &directory) final;

    Core::ShellCommand *createInitialCheckoutCommand(const QString &url,
                                                     const Utils::FilePath &baseDirectory,
                                                     const QString &localName,
                                                     const QStringList &extraArgs) final;

    RepoUrl getRepoUrl(const QString &location) const override;

    QStringList additionalToolsPath() const final;

    bool isCommitEditorOpen() const;
    void startCommit(CommitType commitType = SimpleCommit);
    void updateBranches(const QString &repository);
    void updateCurrentBranch();

    void manageRemotes();
    void initRepository();
    void startRebaseFromCommit(const QString &workingDirectory, QString commit);

    void updateActions(VcsBase::VcsBasePluginPrivate::ActionState) override;
    bool submitEditorAboutToClose() override;

    void diffCurrentFile();
    void diffCurrentProject();
    void commitFromEditor() override;
    void logFile();
    void blameFile();
    void logProject();
    void logRepository();
    void undoFileChanges(bool revertStaging);
    void resetRepository();
    void recoverDeletedFiles();
    void startRebase();
    void startChangeRelatedAction(const Core::Id &id);
    void stageFile();
    void unstageFile();
    void gitkForCurrentFile();
    void gitkForCurrentFolder();
    void gitGui();
    void cleanProject();
    void cleanRepository();
    void updateSubmodules();
    void applyCurrentFilePatch();
    void promptApplyPatch();

    void stash(bool unstagedOnly = false);
    void stashUnstaged();
    void stashSnapshot();
    void stashPop();
    void branchList();
    void stashList();
    void fetch();
    void pull();
    void push();
    void startMergeTool();
    void continueOrAbortCommand();
    void updateContinueAndAbortCommands();
    void delayedPushToGerrit();

    Core::Command *createCommand(QAction *action, Core::ActionContainer *ac, Core::Id id,
                                 const Core::Context &context, bool addToLocator,
                                 const std::function<void()> &callback, const QKeySequence &keys);

    Utils::ParameterAction *createParameterAction(Core::ActionContainer *ac,
                                                  const QString &defaultText, const QString &parameterText,
                                                  Core::Id id, const Core::Context &context, bool addToLocator,
                                                  const std::function<void()> &callback,
                                                  const QKeySequence &keys = QKeySequence());

    QAction *createFileAction(Core::ActionContainer *ac,
                              const QString &defaultText, const QString &parameterText,
                              Core::Id id, const Core::Context &context, bool addToLocator,
                              const std::function<void()> &callback,
                              const QKeySequence &keys = QKeySequence());

    QAction *createProjectAction(Core::ActionContainer *ac,
                                 const QString &defaultText, const QString &parameterText,
                                 Core::Id id, const Core::Context &context, bool addToLocator,
                                 void (GitPluginPrivate::*func)(),
                                 const QKeySequence &keys = QKeySequence());

    QAction *createRepositoryAction(Core::ActionContainer *ac, const QString &text, Core::Id id,
                                    const Core::Context &context, bool addToLocator,
                                    const std::function<void()> &callback,
                                    const QKeySequence &keys = QKeySequence());
    QAction *createRepositoryAction(Core::ActionContainer *ac, const QString &text, Core::Id id,
                                    const Core::Context &context, bool addToLocator,
                                    GitClientMemberFunc, const QKeySequence &keys = QKeySequence());

    QAction *createChangeRelatedRepositoryAction(const QString &text, Core::Id id,
                                                 const Core::Context &context);

    void updateRepositoryBrowserAction();
    Core::IEditor *openSubmitEditor(const QString &fileName, const CommitData &cd);
    void cleanCommitMessageFile();
    void cleanRepository(const QString &directory);
    void applyPatch(const QString &workingDirectory, QString file = QString());
    void updateVersionWarning();


    void onApplySettings();;
    void describe(const QString &source, const QString &id) { m_gitClient.show(source, id); };

    Core::CommandLocator *m_commandLocator = nullptr;

    QAction *m_menuAction = nullptr;
    QAction *m_repositoryBrowserAction = nullptr;
    QAction *m_mergeToolAction = nullptr;
    QAction *m_submoduleUpdateAction = nullptr;
    QAction *m_abortMergeAction = nullptr;
    QAction *m_abortRebaseAction = nullptr;
    QAction *m_abortCherryPickAction = nullptr;
    QAction *m_abortRevertAction = nullptr;
    QAction *m_skipRebaseAction = nullptr;
    QAction *m_continueRebaseAction = nullptr;
    QAction *m_continueCherryPickAction = nullptr;
    QAction *m_continueRevertAction = nullptr;
    QAction *m_fixupCommitAction = nullptr;
    QAction *m_interactiveRebaseAction = nullptr;

    QVector<Utils::ParameterAction *> m_fileActions;
    QVector<Utils::ParameterAction *> m_projectActions;
    QVector<QAction *> m_repositoryActions;
    Utils::ParameterAction *m_applyCurrentFilePatchAction = nullptr;
    Gerrit::Internal::GerritPlugin *m_gerritPlugin = nullptr;

    GitSettings m_settings;
    GitClient m_gitClient{&m_settings};
    QPointer<StashDialog> m_stashDialog;
    BranchViewFactory m_branchViewFactory;
    QPointer<RemoteDialog> m_remoteDialog;
    QString m_submitRepository;
    QString m_commitMessageFileName;
    bool m_submitActionTriggered = false;

    GitSettingsPage settingPage{&m_settings, std::bind(&GitPluginPrivate::onApplySettings, this)};

    GitGrep gitGrep{&m_gitClient};

    VcsEditorFactory commandLogEditorFactory {
        &commandLogEditorParameters,
        [] { return new GitEditorWidget; },
        std::bind(&GitPluginPrivate::describe, this, _1, _2)
    };

    VcsEditorFactory logEditorFactory {
        &logEditorParameters,
        [] { return new GitLogEditorWidget; },
        std::bind(&GitPluginPrivate::describe, this, _1, _2)
    };

    VcsEditorFactory blameEditorFactory {
        &blameEditorParameters,
        [] { return new GitEditorWidget; },
        std::bind(&GitPluginPrivate::describe, this, _1, _2)
    };

    VcsEditorFactory commitTextEditorFactory {
        &commitTextEditorParameters,
        [] { return new GitEditorWidget; },
        std::bind(&GitPluginPrivate::describe, this, _1, _2)
    };

    VcsEditorFactory rebaseEditorFactory {
        &rebaseEditorParameters,
        [] { return new GitEditorWidget; },
        std::bind(&GitPluginPrivate::describe, this, _1, _2)
    };

    VcsSubmitEditorFactory submitEditorFactory {
        submitParameters,
        [] { return new GitSubmitEditor; },
        this
    };
};

void GitPluginPrivate::onApplySettings()
{
    configurationChanged();
    updateRepositoryBrowserAction();
    bool gitFoundOk;
    QString errorMessage;
    m_settings.gitExecutable(&gitFoundOk, &errorMessage);
    if (!gitFoundOk)
        Core::AsynchronousMessageBox::warning(tr("Git Settings"), errorMessage);
}

GitClient *GitPlugin::client()
{
    return &dd->m_gitClient;
}

IVersionControl *GitPlugin::versionControl()
const GitSettings &GitPlugin::settings()
    return dd->m_settings;
const VcsBasePluginState &GitPlugin::currentState()
{
    return dd->currentState();
}

QString GitPlugin::msgRepositoryLabel(const QString &repository)
QString GitPlugin::invalidBranchAndRemoteNamePattern()
        (m_gitClient.*func)(currentState().topLevel());
    : VcsBase::VcsBasePluginPrivate(Context(Constants::GIT_CONTEXT))

    setTopicCache(new GitTopicCache(&m_gitClient));

    connect(VcsOutputWindow::instance(), &VcsOutputWindow::referenceClicked,
            this, [this](const QString &name) {
        const VcsBasePluginState state = currentState();
        QTC_ASSERT(state.hasTopLevel(), return);
        m_gitClient.show(state.topLevel(), name);
    });

    m_gitClient.diffFile(state.currentFileTopLevel(), state.relativeCurrentFile());
        m_gitClient.diffRepository(state.currentProjectTopLevel());
        m_gitClient.diffProject(state.currentProjectTopLevel(), relativeProject);
    m_gitClient.log(state.currentFileTopLevel(), state.relativeCurrentFile(), true);
    VcsBaseEditorWidget *editor = m_gitClient.annotate(
    m_gitClient.log(state.currentProjectTopLevel(), state.relativeCurrentProject());
    m_gitClient.log(state.topLevel());
    m_gitClient.revert({state.currentFile()}, revertStaging);
        m_gitClient.reset(topLevel, dialog.resetFlag(), dialog.commit());
    m_gitClient.recoverDeletedFiles(state.topLevel());
    if (workingDirectory.isEmpty() || !m_gitClient.canRebase(workingDirectory))
    if (m_gitClient.beginStashScope(workingDirectory, "Rebase-i"))
        m_gitClient.interactiveRebase(workingDirectory, commit, false);
        m_gitClient.show(workingDirectory, change);
        m_gitClient.archive(workingDirectory, change);
        m_gitClient.synchronousCherryPick(workingDirectory, change);
        m_gitClient.synchronousRevert(workingDirectory, change);
        m_gitClient.checkout(workingDirectory, change);
    m_gitClient.addFile(state.currentFileTopLevel(), state.relativeCurrentFile());
    m_gitClient.synchronousReset(state.currentFileTopLevel(), {state.relativeCurrentFile()});
    m_gitClient.launchGitK(state.currentFileTopLevel(), state.relativeCurrentFile());
     *  m_gitClient.launchGitK(dir.currentFileDirectory(), ".");
        m_gitClient.launchGitK(state.currentFileDirectory());
        m_gitClient.launchGitK(dir.absolutePath(), folderName);
    m_gitClient.launchGitGui(state.topLevel());
    if (!m_gitClient.getCommitData(state.topLevel(), &commitTemplate, data, &errorMessage)) {
    unsigned version = m_gitClient.gitVersion();
        if (!m_gitClient.addAndCommit(m_submitRepository, editor->panelData(), commitType,
        if (!m_gitClient.beginStashScope(m_submitRepository, "Rebase-fixup",
        m_gitClient.interactiveRebase(m_submitRepository, amendSHA1, true);
        m_gitClient.continueCommandIfNeeded(m_submitRepository);
            m_gitClient.push(m_submitRepository);
    m_gitClient.fetch(currentState().topLevel(), QString());
    bool rebase = m_settings.boolValue(GitSettings::pullRebaseKey);
        QString currentBranch = m_gitClient.synchronousCurrentLocalBranch(topLevel);
            rebase = (m_gitClient.readConfigValue(topLevel, currentBranch) == "true");
    if (!m_gitClient.beginStashScope(topLevel, "Pull", rebase ? Default : AllowUnstashed))
    m_gitClient.pull(topLevel, rebase);
    m_gitClient.push(state.topLevel());
    m_gitClient.merge(state.topLevel());
        m_gitClient.synchronousMerge(state.topLevel(), "--abort");
        m_gitClient.rebase(state.topLevel(), "--abort");
        m_gitClient.synchronousCherryPick(state.topLevel(), "--abort");
        m_gitClient.synchronousRevert(state.topLevel(), "--abort");
        m_gitClient.rebase(state.topLevel(), "--skip");
        m_gitClient.rebase(state.topLevel(), "--continue");
        m_gitClient.cherryPick(state.topLevel(), "--continue");
        m_gitClient.revert(state.topLevel(), "--continue");
    const bool gotFiles = m_gitClient.synchronousCleanList(directory, QString(), &files, &ignoredFiles, &errorMessage);
    m_gitClient.updateSubmodulesIfNeeded(state.topLevel(), false);
    if (!m_gitClient.beginStashScope(workingDirectory, "Apply-Patch", AllowUnstashed))
            m_gitClient.endStashScope(workingDirectory);
    if (m_gitClient.synchronousApplyPatch(workingDirectory, file, &errorMessage)) {
    m_gitClient.endStashScope(workingDirectory);
    m_gitClient.executeSynchronousStash(topLevel, QString(), unstagedOnly);
    const QString id = m_gitClient.synchronousStash(state.topLevel(), QString(),
    m_gitClient.stashPop(repository);
    if (m_branchViewFactory.view())
        m_branchViewFactory.view()->refresh(state.topLevel(), false);
            && !m_gitClient.submoduleList(state.topLevel()).isEmpty());
                m_gitClient.checkCommandInProgress(currentState().topLevel());
    if (m_branchViewFactory.view())
        m_branchViewFactory.view()->refreshIfSame(repository);
    if (m_branchViewFactory.view())
        m_branchViewFactory.view()->refreshCurrentBranch();
    if (!GitPlugin::client() || options.size() < 2)
        GitPlugin::client()->show(workingDirectory, options.at(1));
            = !m_settings.stringValue(GitSettings::repositoryBrowserCmd).isEmpty();
QString GitPluginPrivate::displayName() const
{
    return QLatin1String("Git");
}

Core::Id GitPluginPrivate::id() const
{
    return Core::Id(VcsBase::Constants::VCS_ID_GIT);
}

bool GitPluginPrivate::isVcsFileOrDirectory(const Utils::FilePath &fileName) const
{
    if (fileName.fileName().compare(".git", Utils::HostOsInfo::fileNameCaseSensitivity()))
        return false;
    if (fileName.isDir())
        return true;
    QFile file(fileName.toString());
    if (!file.open(QFile::ReadOnly))
        return false;
    return file.read(8) == "gitdir: ";
}

bool GitPluginPrivate::isConfigured() const
{
    return !m_gitClient.vcsBinary().isEmpty();
}

bool GitPluginPrivate::supportsOperation(Operation operation) const
{
    if (!isConfigured())
        return false;

    switch (operation) {
    case AddOperation:
    case DeleteOperation:
    case MoveOperation:
    case CreateRepositoryOperation:
    case SnapshotOperations:
    case AnnotateOperation:
    case InitialCheckoutOperation:
        return true;
    }
    return false;
}

bool GitPluginPrivate::vcsOpen(const QString & /*fileName*/)
{
    return false;
}

bool GitPluginPrivate::vcsAdd(const QString & fileName)
{
    const QFileInfo fi(fileName);
    return m_gitClient.synchronousAdd(fi.absolutePath(), {fi.fileName()});
}

bool GitPluginPrivate::vcsDelete(const QString & fileName)
{
    const QFileInfo fi(fileName);
    return m_gitClient.synchronousDelete(fi.absolutePath(), true, {fi.fileName()});
}

bool GitPluginPrivate::vcsMove(const QString &from, const QString &to)
{
    const QFileInfo fromInfo(from);
    const QFileInfo toInfo(to);
    return m_gitClient.synchronousMove(fromInfo.absolutePath(), fromInfo.absoluteFilePath(), toInfo.absoluteFilePath());
}

bool GitPluginPrivate::vcsCreateRepository(const QString &directory)
{
    return m_gitClient.synchronousInit(directory);
}

QString GitPluginPrivate::vcsTopic(const QString &directory)
{
    QString topic = Core::IVersionControl::vcsTopic(directory);
    const QString commandInProgress = m_gitClient.commandInProgressDescription(directory);
    if (!commandInProgress.isEmpty())
        topic += " (" + commandInProgress + ')';
    return topic;
}

Core::ShellCommand *GitPluginPrivate::createInitialCheckoutCommand(const QString &url,
                                                                    const Utils::FilePath &baseDirectory,
                                                                    const QString &localName,
                                                                    const QStringList &extraArgs)
{
    QStringList args = {"clone", "--progress"};
    args << extraArgs << url << localName;

    auto command = new VcsBase::VcsCommand(baseDirectory.toString(), m_gitClient.processEnvironment());
    command->addFlags(VcsBase::VcsCommand::SuppressStdErr);
    command->addJob({m_gitClient.vcsBinary(), args}, -1);
    return command;
}

GitPluginPrivate::RepoUrl GitPluginPrivate::getRepoUrl(const QString &location) const
{
    return GitRemote(location);
}

QStringList GitPluginPrivate::additionalToolsPath() const
{
    QStringList res = m_gitClient.settings().searchPathList();
    const QString binaryPath = m_gitClient.gitBinDirectory().toString();
    if (!binaryPath.isEmpty() && !res.contains(binaryPath))
        res << binaryPath;
    return res;
}

bool GitPluginPrivate::managesDirectory(const QString &directory, QString *topLevel) const
{
    const QString topLevelFound = m_gitClient.findRepositoryForDirectory(directory);
    if (topLevel)
        *topLevel = topLevelFound;
    return !topLevelFound.isEmpty();
}

bool GitPluginPrivate::managesFile(const QString &workingDirectory, const QString &fileName) const
{
    return m_gitClient.managesFile(workingDirectory, fileName);
}

QStringList GitPluginPrivate::unmanagedFiles(const QString &workingDir,
                                              const QStringList &filePaths) const
{
    return m_gitClient.unmanagedFiles(workingDir, filePaths);
}

bool GitPluginPrivate::vcsAnnotate(const QString &file, int line)
{
    const QFileInfo fi(file);
    m_gitClient.annotate(fi.absolutePath(), fi.fileName(), QString(), line);
    return true;
}

void GitPlugin::emitFilesChanged(const QStringList &l)
{
    emit dd->filesChanged(l);
}

void GitPlugin::emitRepositoryChanged(const QString &r)
{
    emit dd->repositoryChanged(r);
}

void GitPlugin::startRebaseFromCommit(const QString &workingDirectory, const QString &commit)
    dd->startRebaseFromCommit(workingDirectory, commit);
}

void GitPlugin::manageRemotes()
{
    dd->manageRemotes();
}

void GitPlugin::initRepository()
{
    dd->initRepository();
}

void GitPlugin::startCommit()
{
    dd->startCommit();
}

void GitPlugin::updateCurrentBranch()
{
    dd->updateCurrentBranch();
}

void GitPlugin::updateBranches(const QString &repository)
{
    dd->updateBranches(repository);
}

void GitPlugin::gerritPush(const QString &topLevel)
{
    dd->m_gerritPlugin->push(topLevel);
}

bool GitPlugin::isCommitEditorOpen()
{
    return dd->isCommitEditorOpen();
    VcsBaseEditorWidget::testDiffFileResolving(dd->commitTextEditorFactory);
    VcsBaseEditorWidget::testLogResolving(dd->logEditorFactory, data,

#include "gitplugin.moc"