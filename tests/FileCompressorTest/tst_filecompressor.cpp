#include <QtTest>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QRandomGenerator>

#include "core/FileCompressor.h"

class FileCompressorTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // In-memory tests
    void gzip_emptyData_returnsEmpty();
    void gzip_smallData_compressesAndDecompresses();
    void gzip_largeData_compressesAndDecompresses();
    void gzip_binaryData_compressesAndDecompresses();
    void gunzip_invalidData_returnsEmpty();
    void gunzip_truncatedData_returnsEmpty();

    // File-based tests
    void gzipFile_smallFile_compressesAndDecompresses();
    void gzipFile_largeFile_compressesAndDecompresses();
    void gzipFile_nonExistentSource_returnsFalse();
    void gzipFile_invalidDestPath_returnsFalse();
    void gzipFile_progressCallback_isCalled();
    void gzipFile_progressCallbackCancel_stopsCompression();

    // Benchmarks
    void gzip_benchmark_compression();
    void gzip_benchmark_decompression();

private:
    QByteArray generateRandomData(int size);
    QByteArray generateCompressibleData(int size);

    QTemporaryDir *tempDir = nullptr;
};

void FileCompressorTest::initTestCase()
{
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
}

void FileCompressorTest::cleanupTestCase()
{
    delete tempDir;
    tempDir = nullptr;
}

QByteArray FileCompressorTest::generateRandomData(int size)
{
    QByteArray data(size, '\0');
    for (int i = 0; i < size; ++i)
        data[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    return data;
}

QByteArray FileCompressorTest::generateCompressibleData(int size)
{
    // Generate data with repeated patterns (highly compressible)
    QByteArray pattern = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QByteArray data;
    data.reserve(size);
    while (data.size() < size)
        data.append(pattern);
    return data.left(size);
}

// ============================================================================
// In-memory tests
// ============================================================================

void FileCompressorTest::gzip_emptyData_returnsEmpty()
{
    QByteArray empty;
    QByteArray compressed = FileCompressor::gzip(empty);
    QVERIFY(compressed.isEmpty());

    QByteArray decompressed = FileCompressor::gunzip(empty);
    QVERIFY(decompressed.isEmpty());
}

void FileCompressorTest::gzip_smallData_compressesAndDecompresses()
{
    QByteArray original = "Hello, World! This is a test message for compression.";

    QByteArray compressed = FileCompressor::gzip(original);
    QVERIFY(!compressed.isEmpty());
    QVERIFY(compressed != original);

    QByteArray decompressed = FileCompressor::gunzip(compressed);
    QCOMPARE(decompressed, original);
}

void FileCompressorTest::gzip_largeData_compressesAndDecompresses()
{
    // 1 MB of compressible data
    QByteArray original = generateCompressibleData(1024 * 1024);

    QByteArray compressed = FileCompressor::gzip(original);
    QVERIFY(!compressed.isEmpty());
    // Compressible data should be significantly smaller
    QVERIFY(compressed.size() < original.size() / 2);

    QByteArray decompressed = FileCompressor::gunzip(compressed);
    QCOMPARE(decompressed, original);
}

void FileCompressorTest::gzip_binaryData_compressesAndDecompresses()
{
    // Random binary data (less compressible)
    QByteArray original = generateRandomData(10000);

    QByteArray compressed = FileCompressor::gzip(original);
    QVERIFY(!compressed.isEmpty());

    QByteArray decompressed = FileCompressor::gunzip(compressed);
    QCOMPARE(decompressed, original);
}

void FileCompressorTest::gunzip_invalidData_returnsEmpty()
{
    QByteArray invalid = "This is not gzip compressed data";
    QByteArray result = FileCompressor::gunzip(invalid);
    QVERIFY(result.isEmpty());
}

void FileCompressorTest::gunzip_truncatedData_returnsEmpty()
{
    QByteArray original = "Test data for truncation test";
    QByteArray compressed = FileCompressor::gzip(original);
    QVERIFY(!compressed.isEmpty());

    // Truncate the compressed data
    QByteArray truncated = compressed.left(compressed.size() / 2);
    QByteArray result = FileCompressor::gunzip(truncated);
    // Should return empty or partial data, but not crash
    QVERIFY(result != original);
}

// ============================================================================
// File-based tests
// ============================================================================

void FileCompressorTest::gzipFile_smallFile_compressesAndDecompresses()
{
    QString sourceFile = tempDir->filePath("small_source.txt");
    QString compressedFile = tempDir->filePath("small_compressed.gz");
    QString decompressedFile = tempDir->filePath("small_decompressed.txt");

    // Create source file
    QByteArray original = "Small file content for compression test.\n";
    {
        QFile file(sourceFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(original);
    }

    // Compress
    QVERIFY(FileCompressor::gzipFile(sourceFile, compressedFile));
    QVERIFY(QFile::exists(compressedFile));

    // Decompress
    QVERIFY(FileCompressor::gunzipFile(compressedFile, decompressedFile));
    QVERIFY(QFile::exists(decompressedFile));

    // Verify content
    QFile resultFile(decompressedFile);
    QVERIFY(resultFile.open(QIODevice::ReadOnly));
    QCOMPARE(resultFile.readAll(), original);
}

void FileCompressorTest::gzipFile_largeFile_compressesAndDecompresses()
{
    QString sourceFile = tempDir->filePath("large_source.bin");
    QString compressedFile = tempDir->filePath("large_compressed.gz");
    QString decompressedFile = tempDir->filePath("large_decompressed.bin");

    // Create 5 MB source file with compressible data
    QByteArray original = generateCompressibleData(5 * 1024 * 1024);
    {
        QFile file(sourceFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(original);
    }

    // Compress
    QVERIFY(FileCompressor::gzipFile(sourceFile, compressedFile));
    QVERIFY(QFile::exists(compressedFile));

    // Compressed file should be smaller
    QFileInfo compressedInfo(compressedFile);
    QVERIFY(compressedInfo.size() < original.size() / 2);

    // Decompress
    QVERIFY(FileCompressor::gunzipFile(compressedFile, decompressedFile));

    // Verify content
    QFile resultFile(decompressedFile);
    QVERIFY(resultFile.open(QIODevice::ReadOnly));
    QCOMPARE(resultFile.readAll(), original);
}

void FileCompressorTest::gzipFile_nonExistentSource_returnsFalse()
{
    QString sourceFile = tempDir->filePath("non_existent_file.txt");
    QString compressedFile = tempDir->filePath("output.gz");

    QVERIFY(!FileCompressor::gzipFile(sourceFile, compressedFile));
    QVERIFY(!QFile::exists(compressedFile));
}

void FileCompressorTest::gzipFile_invalidDestPath_returnsFalse()
{
    QString sourceFile = tempDir->filePath("source_for_invalid_dest.txt");
    QString invalidDest = "/nonexistent/path/output.gz";

    // Create source file
    {
        QFile file(sourceFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("test");
    }

    QVERIFY(!FileCompressor::gzipFile(sourceFile, invalidDest));
}

void FileCompressorTest::gzipFile_progressCallback_isCalled()
{
    QString sourceFile = tempDir->filePath("progress_source.bin");
    QString compressedFile = tempDir->filePath("progress_compressed.gz");

    // Create 1 MB file
    QByteArray original = generateCompressibleData(1024 * 1024);
    {
        QFile file(sourceFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(original);
    }

    int callCount = 0;
    qint64 lastProcessed = 0;
    bool progressValid = true;

    auto progressCallback = [&](qint64 processed, qint64 total) -> bool {
        ++callCount;
        if (processed < lastProcessed || total <= 0)
            progressValid = false;
        lastProcessed = processed;
        return true;  // Continue
    };

    QVERIFY(FileCompressor::gzipFile(sourceFile, compressedFile, progressCallback));
    QVERIFY(callCount > 0);
    QVERIFY(progressValid);
}

void FileCompressorTest::gzipFile_progressCallbackCancel_stopsCompression()
{
    QString sourceFile = tempDir->filePath("cancel_source.bin");
    QString compressedFile = tempDir->filePath("cancel_compressed.gz");

    // Create 2 MB file
    QByteArray original = generateCompressibleData(2 * 1024 * 1024);
    {
        QFile file(sourceFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(original);
    }

    int callCount = 0;

    auto progressCallback = [&](qint64, qint64) -> bool {
        ++callCount;
        // Cancel after a few calls
        return callCount < 3;
    };

    QVERIFY(!FileCompressor::gzipFile(sourceFile, compressedFile, progressCallback));
    // Destination file should be cleaned up
    QVERIFY(!QFile::exists(compressedFile));
}

// ============================================================================
// Benchmarks
// ============================================================================

void FileCompressorTest::gzip_benchmark_compression()
{
    QByteArray data = generateCompressibleData(1024 * 1024);  // 1 MB

    QByteArray compressed;
    QBENCHMARK {
        compressed = FileCompressor::gzip(data);
    }

    QVERIFY(!compressed.isEmpty());
}

void FileCompressorTest::gzip_benchmark_decompression()
{
    QByteArray data = generateCompressibleData(1024 * 1024);  // 1 MB
    QByteArray compressed = FileCompressor::gzip(data);

    QByteArray decompressed;
    QBENCHMARK {
        decompressed = FileCompressor::gunzip(compressed);
    }

    QCOMPARE(decompressed, data);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    FileCompressorTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_filecompressor.moc"
