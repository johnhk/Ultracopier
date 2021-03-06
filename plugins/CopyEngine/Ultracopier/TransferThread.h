/** \file TransferThread.h
\brief Thread changed to manage the inode operation, the signals, canceling, pre and post operations
\author alpha_one_x86
\licence GPL3, see the file COPYING */

#ifndef TRANSFERTHREAD_H
#define TRANSFERTHREAD_H

#include <QThread>
#include <QFileInfo>
#include <QString>
#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QDir>
#include <QPair>

#ifdef Q_OS_UNIX
    #include <utime.h>
    #include <time.h>
    #include <unistd.h>
    #include <sys/stat.h>
#else
    #ifdef Q_OS_WIN32
        #ifdef ULTRACOPIER_PLUGIN_SET_TIME_UNIX_WAY
            #include <utime.h>
            #include <time.h>
            #include <unistd.h>
            #include <sys/stat.h>
        #endif
    #endif
#endif

#include "ReadThread.h"
#include "WriteThread.h"
#include "Environment.h"
#include "DriveManagement.h"
#include "StructEnumDefinition_CopyEngine.h"

/// \brief Thread changed to manage the inode operation, the signals, canceling, pre and post operations
class TransferThread : public QThread
{
    Q_OBJECT
public:
    explicit TransferThread();
    ~TransferThread();
    /// \brief get transfer stat
    TransferStat getStat() const;
    #ifdef ULTRACOPIER_PLUGIN_DEBUG
    /// \brief to set the id
    void setId(int id);
    /// \brief get the reading letter
    QChar readingLetter() const;
    /// \brief get the writing letter
    QChar writingLetter() const;
    #endif
    /// \brief to have semaphore, and try create just one by one
    void setMkpathTransfer(QSemaphore *mkpathTransfer);
    /// \brief to store the transfer id
    quint64			transferId;
    /// \brief to store the transfer size
    quint64			transferSize;

    void set_doChecksum(bool doChecksum);
    void set_checksumIgnoreIfImpossible(bool checksumIgnoreIfImpossible);
    void set_checksumOnlyOnError(bool checksumOnlyOnError);
    void set_osBuffer(bool osBuffer);
    void set_osBufferLimited(bool osBufferLimited);

    //not copied size, because that's count to the checksum, ...
    quint64 realByteTransfered() const;
    QPair<quint64,quint64> progression() const;
    static QString resolvedName(const QFileInfo &inode);
    QString getSourcePath() const;
    QString getDestinationPath() const;
    QFileInfo getSourceInode() const;
    QFileInfo getDestinationInode() const;
    Ultracopier::CopyMode getMode() const;
protected:
    void run();
signals:
    //to send state
    void preOperationStopped() const;
    void checkIfItCanBeResumed() const;
    //void transferStarted();//not sended (and not used then)
    void readStopped() const;
    void writeStopped() const;
    void postOperationStopped() const;
    //get dialog
    void fileAlreadyExists(const QFileInfo &info,const QFileInfo &info2,const bool &isSame) const;
    void errorOnFile(const QFileInfo &info,const QString &string,const ErrorType &errorType=ErrorType_Normal) const;
    //internal signal
    void internalStartPostOperation() const;
    void internalStartPreOperation() const;
    void internalStartResumeAfterErrorAndSeek() const;
    /// \brief To debug source
    void debugInformation(const Ultracopier::DebugLevel &level,QString fonction,QString text,QString file,int ligne) const;
    void tryPutAtBottom() const;
    //force into the right thread
    void internalTryStartTheTransfer() const;
    /// \brief update the transfer stat
    void pushStat(const TransferStat &stat,const quint64 &pos) const;
public slots:
    /// \brief to start the transfer of data
    void startTheTransfer();
    /// \brief to set files to transfer
    bool setFiles(const QFileInfo& source,const qint64 &size,const QFileInfo& destination,const Ultracopier::CopyMode &mode);
    /// \brief to set file exists action to do
    void setFileExistsAction(const FileExistsAction &action);
    /// \brief to set the new name of the destination
    void setFileRename(const QString &nameForRename);
    /// \brief to start the transfer of data
    void setAlwaysFileExistsAction(const FileExistsAction &action);
    /// \brief set the copy info and options before runing
    void setRightTransfer(const bool doRightTransfer);
    /// \brief set keep date
    void setKeepDate(const bool keepDate);
    #ifdef ULTRACOPIER_PLUGIN_SPEED_SUPPORT
    /// \brief set the current max speed in KB/s
    void setMultiForBigSpeed(const int &maxSpeed);
    #endif
    /// \brief set block size in KB
    bool setBlockSize(const unsigned int blockSize);
    /// \brief pause the copy
    void pause();
    /// \brief resume the copy
    void resume();
    /// \brief stop the copy
    void stop();
    /// \brief skip the copy
    void skip();
    /// \brief retry after error
    void retryAfterError();
    /// \brief return info about the copied size
    qint64 copiedSize();
    /// \brief put the current file at bottom
    void putAtBottom();

    void setDrive(const QStringList &mountSysPoint,const QList<QStorageInfo::DriveType> &driveType);

    void set_osBufferLimit(const unsigned int &osBufferLimit);
    void setRenamingRules(const QString &firstRenamingRule,const QString &otherRenamingRule);
    #ifdef ULTRACOPIER_PLUGIN_SPEED_SUPPORT
    //speed limitation
    void timeOfTheBlockCopyFinished();
    #endif

    bool setParallelBuffer(const int &parallelBuffer);
    bool setSequentialBuffer(const int &sequentialBuffer);
    void setTransferAlgorithm(const TransferAlgorithm &transferAlgorithm);
    void setDeletePartiallyTransferredFiles(const bool &deletePartiallyTransferredFiles);
    void setRenameTheOriginalDestination(const bool &renameTheOriginalDestination);
private slots:
    void preOperation();
    void readIsReady();
    void writeIsReady();
    void readIsFinish();
    void writeIsFinish();
    void readIsClosed();
    void writeIsClosed();
    void postOperation();
    void getWriteError();
    void getReadError();
    void readChecksumFinish(const QByteArray&);
    void writeChecksumFinish(const QByteArray&);
    void compareChecksum();
    //void syncAfterErrorAndReadFinish();
    void readThreadIsSeekToZeroAndWait();
    void writeThreadIsReopened();
    void readThreadResumeAfterError();
    //to filter the emition of signal
    void readIsStopped();
    void writeIsStopped();
    //force into the right thread
    void internalStartTheTransfer();
private:
    enum MoveReturn
    {
        MoveReturn_skip=0,
        MoveReturn_moved=1,
        MoveReturn_error=2
    };
    TransferStat	transfer_stat;
    ReadThread		readThread;
    WriteThread		writeThread;
    /*QString			source;
    QString			destination;*/
    Ultracopier::CopyMode		mode;
    bool			doRightTransfer;
    bool			keepDate;
    //ready = open + ready to operation (no error to resolv)
    bool			readIsReadyVariable;
    bool			writeIsReadyVariable;
    //can be open but with error
    bool            readIsOpeningVariable;//after call open() and before the end of internalOpen(), mostly to prevent internal error by open() when another is running
    bool            writeIsOpeningVariable;//after call open() and before the end of internalOpen(), mostly to prevent internal error by open() when another is running
    bool			readIsOpenVariable;
    bool			writeIsOpenVariable;
    bool			readIsFinishVariable;
    bool			writeIsFinishVariable;
    bool			readIsClosedVariable;
    bool			writeIsClosedVariable;
    bool			canBeMovedDirectlyVariable,canBeCopiedDirectlyVariable;
    DriveManagement driveManagement;
    QByteArray		sourceChecksum,destinationChecksum;
    volatile bool	stopIt;
    volatile bool	canStartTransfer;
    bool			retry;
    QFileInfo		source;
    QFileInfo		destination;
    qint64			size;
    FileExistsAction	fileExistsAction;
    FileExistsAction	alwaysDoFileExistsAction;
    bool			needSkip,needRemove;
    QDateTime		maxTime;
    int             id;
    QSemaphore		*mkpathTransfer;
    bool			doChecksum,real_doChecksum;
    bool			checksumIgnoreIfImpossible;
    bool			checksumOnlyOnError;
    bool            deletePartiallyTransferredFiles;
    bool			osBuffer;
    bool			osBufferLimited;
    unsigned int	osBufferLimit;
    QString			firstRenamingRule;
    QString			otherRenamingRule;
    //error management
    bool			writeError,writeError_source_seeked,writeError_destination_reopened;
    bool			readError;
    bool            renameTheOriginalDestination;
    bool			fileContentError;
    bool            doTheDateTransfer;
    int             parallelBuffer;
    int             sequentialBuffer;
    int             parallelizeIfSmallerThan;
    TransferAlgorithm transferAlgorithm;
    #ifdef Q_OS_UNIX
            utimbuf butime;
    #else
        #ifdef Q_OS_WIN32
            #ifdef ULTRACOPIER_PLUGIN_SET_TIME_UNIX_WAY
                utimbuf butime;
            #else
                quint32 ftCreateL, ftAccessL, ftWriteL;
                quint32 ftCreateH, ftAccessH, ftWriteH;
            #endif
        #endif
    #endif
    //different pre-operation
    bool isSame();
    bool destinationExists();
    bool checkAlwaysRename();///< return true if has been renamed
    bool canBeMovedDirectly() const;
    bool canBeCopiedDirectly() const;
    void tryMoveDirectly();
    void tryCopyDirectly();
    void ifCanStartTransfer();
    //fonction to edit the file date time
    bool readFileDateTime(const QFileInfo &source);
    bool writeFileDateTime(const QFileInfo &destination);
    void resetExtraVariable();
    //error management function
    void resumeTransferAfterWriteError();
    //to send state
    bool sended_state_preOperationStopped;
    bool sended_state_readStopped;
    bool sended_state_writeStopped;
    //different post-operation
    bool checkIfAllIsClosedAndDoOperations();// return true if all is closed, and do some operations, don't use into condition to check if is closed!
    bool doFilePostOperation();
    //different pre-operation
    void tryOpen();
    bool remainFileOpen() const;
    bool remainSourceOpen() const;
    bool remainDestinationOpen() const;
};

#endif // TRANSFERTHREAD_H
