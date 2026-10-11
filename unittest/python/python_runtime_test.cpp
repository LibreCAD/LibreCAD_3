#include <gtest/gtest.h>

#include <lcpython.h>
#include <pybind11/embed.h>
#include <python_runtime_config.h>

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTemporaryDir>

#include <string>

namespace py = pybind11;

extern std::string lcUnitTestExecutablePath;

namespace {

QString makeInstalledAppDirectory(QTemporaryDir& temporaryDirectory,
                                 const QString& name,
                                 bool withStagedRuntime) {
    const QString prefix = QDir(temporaryDirectory.path()).filePath(name);
    const QString appDirectory = QDir(prefix).filePath("bin");
    const QString packageDirectory =
        QDir(prefix).filePath("share/librecad/lcUIPy");
    if (!QDir().mkpath(appDirectory) ||
        !QDir().mkpath(packageDirectory)) {
        return {};
    }

    QFile packageInit(QDir(packageDirectory).filePath("__init__.py"));
    if (!packageInit.open(QIODevice::WriteOnly) ||
        packageInit.write("# runtime-selection test fixture\n") < 0) {
        return {};
    }
    packageInit.close();

    if (withStagedRuntime) {
        const QString runtimeRoot =
            QDir(prefix).filePath("share/librecad/python");
        const QString stdlib = QDir(runtimeRoot).filePath(
            QString::fromUtf8(LC_PYTHON_STAGED_STDLIB_REL));
        if (!QDir().mkpath(stdlib)) {
            return {};
        }
        QFile osModule(QDir(stdlib).filePath("os.py"));
        if (!osModule.open(QIODevice::WriteOnly) ||
            osModule.write("# runtime-selection test fixture\n") < 0) {
            return {};
        }
    }
    return appDirectory;
}

QProcess::ExitStatus runRuntimeProbe(const QStringList& arguments,
                                    QProcess& process) {
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.remove("PYTHONHOME");
    environment.remove("PYTHONPATH");
    process.setProcessEnvironment(environment);
    process.start(QString::fromStdString(lcUnitTestExecutablePath), arguments);
    if (!process.waitForStarted(5000) || !process.waitForFinished(15000)) {
        return QProcess::CrashExit;
    }
    return process.exitStatus();
}

} // namespace

TEST(PythonRuntime, IsolatedProviderIgnoresAmbientPaths) {
    EXPECT_NO_THROW(lc::python::PythonInit::configureForApplication());
    EXPECT_NO_THROW(lc::python::PythonInit::initialize());

    py::gil_scoped_acquire gil;
    py::exec(R"py(
import json
import lc
import sys

assert sys.flags.isolated == 1
assert sys.flags.ignore_environment == 1
assert sys.flags.utf8_mode == 1
assert '__librecad_python_poison__' not in sys.prefix
assert all('__librecad_python_poison__' not in path for path in sys.path)
assert json.dumps({'ready': True}) == '{"ready": true}'
)py");
}

TEST(PythonRuntime, RuntimeConfigurationIsValidatedInFreshProcesses) {
    ASSERT_FALSE(lcUnitTestExecutablePath.empty());
    QTemporaryDir temporaryDirectory;
    ASSERT_TRUE(temporaryDirectory.isValid());

    const QString missingRuntimeApp = makeInstalledAppDirectory(
        temporaryDirectory, "missing-runtime", false);
    const QString firstStagedApp = makeInstalledAppDirectory(
        temporaryDirectory, "first-staged-runtime", true);
    const QString secondStagedApp = makeInstalledAppDirectory(
        temporaryDirectory, "second-staged-runtime", true);
    ASSERT_FALSE(missingRuntimeApp.isEmpty());
    ASSERT_FALSE(firstStagedApp.isEmpty());
    ASSERT_FALSE(secondStagedApp.isEmpty());

    QProcess missingRuntime;
    EXPECT_EQ(QProcess::NormalExit,
              runRuntimeProbe({"--python-runtime-probe", "missing",
                               missingRuntimeApp}, missingRuntime));
    EXPECT_EQ(2, missingRuntime.exitCode());
    EXPECT_TRUE(QString::fromLocal8Bit(missingRuntime.readAllStandardError())
                    .contains("staged CPython runtime is missing"));

    QProcess repeated;
    EXPECT_EQ(QProcess::NormalExit,
              runRuntimeProbe({"--python-runtime-probe", "repeat",
                               firstStagedApp}, repeated));
    EXPECT_EQ(0, repeated.exitCode());

    QProcess conflict;
    EXPECT_EQ(QProcess::NormalExit,
              runRuntimeProbe({"--python-runtime-probe", "conflict",
                               firstStagedApp, secondStagedApp}, conflict));
    EXPECT_EQ(2, conflict.exitCode());
    EXPECT_TRUE(QString::fromLocal8Bit(conflict.readAllStandardError())
                    .contains("conflicting Python runtime configuration"));

    QProcess lateSelection;
    EXPECT_EQ(QProcess::NormalExit,
              runRuntimeProbe({"--python-runtime-probe", "late",
                               firstStagedApp}, lateSelection));
    EXPECT_EQ(2, lateSelection.exitCode());
    EXPECT_TRUE(QString::fromLocal8Bit(lateSelection.readAllStandardError())
                    .contains("must be selected before initialization"));
}
