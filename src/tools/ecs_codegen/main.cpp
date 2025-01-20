#include <clang-c/Index.h>

#include "stdio.h"
#include <filesystem>
#include <fstream>
#include <iostream>
static constexpr const char* VERSION_STR = "1.0";

struct ClangIndex
{
    ClangIndex(CXIndex index) : val(index) { };
    ~ClangIndex() { clang_disposeIndex(val); }
    CXIndex val;
};

struct ClangString
{
    ClangString(CXString string) : val(string) { };
    ~ClangString() { clang_disposeString(val); }
    const char* c_str() const { return clang_getCString(val); }
    CXString val;
};

std::ostream& operator<<(std::ostream& stream, const ClangString& str)
{
    return stream << str.c_str();
}

struct EcsTypeInformation
{
    std::string filename;
    struct SystemDesc
    {
        std::string name;
    };

    std::vector<SystemDesc> systems;
};

CXChildVisitResult annotationsVisitor(CXCursor cursor, CXCursor, CXClientData clientData)
{
    auto& annotations = *static_cast<std::vector<std::string>*>(clientData);

    if (clang_getCursorKind(cursor) == CXCursor_AnnotateAttr)
    {
        ClangString spelling = clang_getCursorSpelling(cursor);
        std::cout << "Annotation: " << spelling << std::endl;
        annotations.emplace_back(spelling.c_str());
    }
    else
        clang_visitChildren(cursor, annotationsVisitor, clientData);

    return CXChildVisit_Continue;
}

// Function to collect annotations from a cursor and its children
void getAnnotations(CXCursor cursor, std::vector<std::string>& annotations)
{

    std::cout << "Getting annotations for cursor...\n";
    clang_visitChildren(cursor, annotationsVisitor, &annotations);
}

CXChildVisitResult visitor(CXCursor cursor, CXCursor /*parent*/, CXClientData clientData)
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    auto& ecsInfo = *static_cast<EcsTypeInformation*>(clientData);

    if (kind == CXCursor_FunctionDecl)
    {
        ClangString name = clang_getCursorSpelling(cursor);
        CXSourceLocation location = clang_getCursorLocation(cursor);
        CXFile file;
        unsigned line, column, offset;
        clang_getSpellingLocation(location, &file, &line, &column, &offset);

        ClangString fileName = clang_getFileName(file);
        std::cout << "Function: " << name
                  << " at " << (file ? fileName.c_str() : "null")
                  << ":" << line << ":" << column << std::endl;

        std::vector<std::string> annotations;
        getAnnotations(cursor, annotations);

        EcsTypeInformation::SystemDesc systemDesc;
        bool isSystem = false;
        for (auto annotation : annotations)
        {
            if (annotation == "@ecs_system")
            {
                isSystem = true;
                systemDesc.name = name.c_str();
            }
        }

        if (isSystem)
        {
            ecsInfo.systems.push_back(systemDesc);
        }
    }

    return CXChildVisit_Recurse;
}

bool checkDiagnostics(CXTranslationUnit translationUnit)
{
    unsigned int numDiagnostics = clang_getNumDiagnostics(translationUnit);
    if (numDiagnostics == 0)
        return true;

    bool result = true;
    for (unsigned int i = 0; i < numDiagnostics; ++i)
    {
        CXDiagnostic diagnostic = clang_getDiagnostic(translationUnit, i);
        ClangString diagnosticMessage = clang_formatDiagnostic(
            diagnostic, clang_defaultDiagnosticDisplayOptions());
        CXDiagnosticSeverity severity = clang_getDiagnosticSeverity(diagnostic);

        const char* severityStr = nullptr;
        switch (severity)
        {
            case CXDiagnostic_Ignored: severityStr = "Ignored"; break;
            case CXDiagnostic_Note: severityStr = "Note"; break;
            case CXDiagnostic_Warning: severityStr = "Warning"; break;
            case CXDiagnostic_Error: severityStr = "Error"; break;
            case CXDiagnostic_Fatal: severityStr = "Fatal"; break;
        }

        if (severity > CXDiagnostic_Warning)
            result = false;

        std::cout << "[" << severityStr << "] "
                  << diagnosticMessage << std::endl;

        clang_disposeDiagnostic(diagnostic);
    }
    return result;
}

void codegen(const std::string& outputPath, const EcsTypeInformation& ecsTypeInformation)
{
    std::ofstream codegen(outputPath);

    if (!codegen.is_open())
    {
        std::cerr << "Error: Could not open file " << outputPath << " for writing.\n";
        return;
    }

    codegen << "// Generated with ECS Codegen Version: " << VERSION_STR << std::endl;
    codegen << "#include \"" << ecsTypeInformation.filename << '\"' << std::endl;
    auto qw = ecsTypeInformation;
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <source-file>" << " <output-file>\n";
        return 1;
    }
    std::string sourceFile = argv[1];
    std::string outputFile = argv[2];
    if (!std::filesystem::exists(sourceFile))
    {
        std::cerr << "Error: Source file " << sourceFile << " does not exist.\n";
        return 1;
    }

    ClangIndex index = clang_createIndex(0, 0);

    std::vector<const char*> clangArgs = {
        "-x", "c++",
        "-std=c++17",
        "-DECS_CODEGEN"};

    CXTranslationUnit translationUnit;
    CXErrorCode errorCode = clang_parseTranslationUnit2(
        index.val, sourceFile.c_str(), clangArgs.data(), clangArgs.size(), nullptr, 0, CXTranslationUnit_SkipFunctionBodies | CXTranslationUnit_SingleFileParse, &translationUnit);

    switch (errorCode)
    {
        case CXError_Failure:
            std::cerr << "Error: Unable to parse translation unit." << std::endl;
            return -1;
        case CXError_Crashed:
            std::cerr << "Error: libclang crashed :(" << std::endl;
            return -1;
        case CXError_InvalidArguments:
            std::cerr << "Error: Invalid arguments" << std::endl;
            return -1;
        case CXError_ASTReadError:
            std::cerr << "Error: AST deserialization error." << std::endl;
            return -1;
        default:
            std::cerr << "Error: Unknown error." << std::endl;
            return -1;
        case CXError_Success: break;
    }

    if (!translationUnit)
    {
        std::cerr << "Error: Unable to parse translation unit." << std::endl;
        return -1;
    }

    if(!checkDiagnostics(translationUnit))
        return -1;

    CXCursor rootCursor = clang_getTranslationUnitCursor(translationUnit);

    EcsTypeInformation ecsInfo;
    ecsInfo.filename = std::filesystem::path(sourceFile).filename().u8string();
    clang_visitChildren(rootCursor, visitor, &ecsInfo);
    codegen(outputFile, ecsInfo);

    clang_disposeTranslationUnit(translationUnit);
    return 0;
}