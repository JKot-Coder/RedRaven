//
// TODO remove Slange
// TODO change format
// TODO remove unused
//
// The file is meant to be included multiple times, to produce different
// pieces of declaration/definition code related to diagnostic messages
//
// Each diagnostic is declared here with:
//
//     DIAGNOSTIC(id, severity, name, messageFormat)
//
// Where `id` is the unique diagnostic ID, `severity` is the default
// severity (from the `Severity` enum), `name` is a name used to refer
// to this diagnostic from code, and `messageFormat` is the default
// (non-localized) message for the diagnostic, with placeholders
// for any arguments.

#ifndef DIAGNOSTIC
#error Need to #define DIAGNOSTIC(...) before including "DiagnosticDefs.h"
#define DIAGNOSTIC(id, severity, name, messageFormat) /* */
#endif

//
// -1 - Notes that decorate another diagnostic.
//

DIAGNOSTIC(-1, Note, alsoSeePipelineDefinition, "also see pipeline definition")
DIAGNOSTIC(-1, Note, implicitParameterMatchingFailedBecauseNameNotAccessible, "implicit parameter matching failed because the component of the same name is not accessible from '$0'.\ncheck if you have declared necessary requirements and properly used the 'public' qualifier.")
DIAGNOSTIC(-1, Note, implicitParameterMatchingFailedBecauseShaderDoesNotDefineComponent, "implicit parameter matching failed because shader '$0' does not define component '$1'.")
DIAGNOSTIC(-1, Note, implicitParameterMatchingFailedBecauseTypeMismatch, "implicit parameter matching failed because the component of the same name does not match parameter type '$0'.")
DIAGNOSTIC(-1, Note, noteShaderIsTargetingPipeine, "shader '$0' is targeting pipeline '$1'")
DIAGNOSTIC(-1, Note, seeDefinitionOf, "see definition of '$0'")
DIAGNOSTIC(-1, Note, seeInterfaceDefinitionOf, "see interface definition of '$0'")
DIAGNOSTIC(-1, Note, seeUsingOf, "see using of '$0'")
DIAGNOSTIC(-1, Note, seeDefinitionOfShader, "see definition of shader '$0'")
DIAGNOSTIC(-1, Note, seeInclusionOf, "see inclusion of '$0'")
DIAGNOSTIC(-1, Note, seeModuleBeingUsedIn, "see module '$0' being used in '$1'")
DIAGNOSTIC(-1, Note, seePipelineRequirementDefinition, "see pipeline requirement definition")
DIAGNOSTIC(-1, Note, seePotentialDefinitionOfComponent, "see potential definition of component '$0'")
DIAGNOSTIC(-1, Note, seePreviousDefinition, "see previous definition")
DIAGNOSTIC(-1, Note, seePreviousDefinitionOf, "see previous definition of '{0}'")
DIAGNOSTIC(-1, Note, seeRequirementDeclaration, "see requirement declaration")
DIAGNOSTIC(-1, Note, doYouForgetToMakeComponentAccessible, "do you forget to make component '$0' acessible from '$1' (missing public qualifier)?")

DIAGNOSTIC(-1, Note, seeDeclarationOf, "see declaration of '$0'")
DIAGNOSTIC(-1, Note, seeOtherDeclarationOf, "see other declaration of '$0'")
DIAGNOSTIC(-1, Note, seePreviousDeclarationOf, "see previous declaration of '$0'")
DIAGNOSTIC(-1, Note, includeOutput, "include $0")

DIAGNOSTIC(-1, Note, expandedFromMacro, "expanded from macro '{0}'")

//
// 0xxxx -  Command line and interaction with host platform APIs.
//

DIAGNOSTIC(1, Error, cannotOpenFile, "cannot open file '{0}' with error '{1}'.")
DIAGNOSTIC(2, Error, cannotFindFile, "cannot find file '$0'.")
DIAGNOSTIC(2, Error, unsupportedCompilerMode, "unsupported compiler mode.")
DIAGNOSTIC(4, Error, cannotWriteOutputFile, "cannot write output file '$0'.")
DIAGNOSTIC(5, Error, failedToLoadDynamicLibrary, "failed to load dynamic library '$0'")
DIAGNOSTIC(6, Error, tooManyOutputPathsSpecified, "$0 output paths specified, but only $1 entry points given")

DIAGNOSTIC(7, Error, noOutputPathSpecifiedForEntryPoint,
           "no output path specified for entry point '$0' (the '-o' option for an entry point must precede the corresponding '-entry')")

DIAGNOSTIC(8, Error, outputPathsImplyDifferentFormats,
           "the output paths '$0' and '$1' require different code-generation targets")

DIAGNOSTIC(10, Error, explicitOutputPathsAndMultipleTargets, "canot use both explicit output paths ('-o') and multiple targets ('-target')")
DIAGNOSTIC(11, Error, glslIsNotSupported, "the Slang compiler does not support GLSL as a source language")
DIAGNOSTIC(12, Error, cannotDeduceSourceLanguage, "can't deduce language for input file '$0'")
DIAGNOSTIC(13, Error, unknownCodeGenerationTarget, "unknown code generation target '$0'")
DIAGNOSTIC(14, Error, unknownProfile, "unknown profile '$0'")
DIAGNOSTIC(15, Error, unknownStage, "unknown stage '$0'")
DIAGNOSTIC(16, Error, unknownPassThroughTarget, "unknown pass-through target '$0'")
DIAGNOSTIC(17, Error, unknownCommandLineOption, "unknown command-line option '$0'")
DIAGNOSTIC(18, Error, unknownFileSystemOption, "unknown file-system option '$0'")
DIAGNOSTIC(19, Error, unknownSourceLanguage, "unknown source language '$0'")

DIAGNOSTIC(20, Error, entryPointsNeedToBeAssociatedWithTranslationUnits, "when using multiple source files, entry points must be specified after their corresponding source file(s)")
DIAGNOSTIC(22, Error, unknownDownstreamCompiler, "unknown downstream compiler '$0'")

DIAGNOSTIC(24, Error, unknownLineDirectiveMode, "unknown '#line' directive mode '$0'")
DIAGNOSTIC(25, Error, unknownFloatingPointMode, "unknown floating-point mode '$0'")
DIAGNOSTIC(26, Error, unknownOptimiziationLevel, "unknown optimization level '$0'")
DIAGNOSTIC(27, Error, unknownDebugInfoLevel, "unknown debug info level '$0'")

DIAGNOSTIC(28, Error, unableToGenerateCodeForTarget, "unable to generate code for target '$0'")

DIAGNOSTIC(30, Warning, sameStageSpecifiedMoreThanOnce, "the stage '$0' was specified more than once for entry point '$1'")
DIAGNOSTIC(31, Error, conflictingStagesForEntryPoint, "conflicting stages have been specified for entry point '$0'")
DIAGNOSTIC(32, Warning, explicitStageDoesntMatchImpliedStage, "the stage specified for entry point '$0' ('$1') does not match the stage implied by the source file name ('$2')")
DIAGNOSTIC(33, Error, stageSpecificationIgnoredBecauseNoEntryPoints, "one or more stages were specified, but no entry points were specified with '-entry'")
DIAGNOSTIC(34, Error, stageSpecificationIgnoredBecauseBeforeAllEntryPoints, "when compiling multiple entry points, any '-stage' options must follow the '-entry' option that they apply to")
DIAGNOSTIC(35, Error, noStageSpecifiedInPassThroughMode, "no stage was specified for entry point '$0'; when using the '-pass-through' option, stages must be fully specified on the command line")

DIAGNOSTIC(40, Warning, sameProfileSpecifiedMoreThanOnce, "the '$0' was specified more than once for target '$0'")
DIAGNOSTIC(41, Error, conflictingProfilesSpecifiedForTarget, "conflicting profiles have been specified for target '$0'")

DIAGNOSTIC(42, Error, profileSpecificationIgnoredBecauseNoTargets, "a '-profile' option was specified, but no target was specified with '-target'")
DIAGNOSTIC(43, Error, profileSpecificationIgnoredBecauseBeforeAllTargets, "when using multiple targets, any '-profile' option must follow the '-target' it applies to")

DIAGNOSTIC(42, Error, targetFlagsIgnoredBecauseNoTargets, "target options were specified, but no target was specified with '-target'")
DIAGNOSTIC(43, Error, targetFlagsIgnoredBecauseBeforeAllTargets, "when using multiple targets, any target options must follow the '-target' they apply to")

DIAGNOSTIC(50, Error, duplicateTargets, "the target '$0' has been specified more than once")

DIAGNOSTIC(60, Error, cannotDeduceOutputFormatFromPath, "cannot infer an output format from the output path '$0'")
DIAGNOSTIC(61, Error, cannotMatchOutputFileToTarget, "no specified '-target' option matches the output path '$0', which implies the '$1' format")

DIAGNOSTIC(70, Error, cannotMatchOutputFileToEntryPoint, "the output path '$0' is not associated with any entry point; a '-o' option for a compiled kernel must follow the '-entry' option for its corresponding entry point")

DIAGNOSTIC(80, Error, duplicateOutputPathsForEntryPointAndTarget, "multiple output paths have been specified entry point '$0' on target '$1'")
DIAGNOSTIC(81, Error, duplicateOutputPathsForTarget, "multiple output paths have been specified for target '$0'")

DIAGNOSTIC(82, Error, unableToWriteReproFile, "unable to write repro file '%0'")
DIAGNOSTIC(83, Error, unableToWriteModuleContainer, "unable to write module container '%0'")
DIAGNOSTIC(84, Error, unableToReadModuleContainer, "unable to read module container '%0'")
DIAGNOSTIC(85, Error, unableToAddReferenceToModuleContainer, "unable to add a reference to a module container")
DIAGNOSTIC(86, Error, unableToCreateModuleContainer, "unable to create module container")

DIAGNOSTIC(87, Error, unableToSetDefaultDownstreamCompiler, "unable to set default downstream compiler for source language '%0' to '%1'")

//
// 001xx - Downstream Compilers
//

DIAGNOSTIC(100, Error, failedToLoadDownstreamCompiler, "failed to load downstream compiler '$0'")
DIAGNOSTIC(99999, Note, noteFailedToLoadDynamicLibrary, "failed to load dynamic library '$0'")

//
// 15xxx - Preprocessing
//

// 150xx - conditionals
DIAGNOSTIC(15000, Error, endOfFileInPreprocessorConditional, "end of file encountered during preprocessor conditional")
DIAGNOSTIC(15001, Error, directiveWithoutIf, "'{0}' directive without '#if'")
DIAGNOSTIC(15002, Error, directiveAfterElse, "'{0}' directive without '#if'")

DIAGNOSTIC(-1, Note, seeDirective, "see '{0}' directive")

// 151xx - directive parsing
DIAGNOSTIC(15100, Error, expectedPreprocessorDirectiveName, "expected preprocessor directive name")
DIAGNOSTIC(15101, Error, unknownPreprocessorDirective, "unknown preprocessor directive '{0}'")
DIAGNOSTIC(15102, Error, expectedTokenInPreprocessorDirective, "expected '{0}' in '{1}' directive")
DIAGNOSTIC(15103, Error, unexpectedTokensAfterDirective, "unexpected tokens following '{0}' directive")

// 152xx - preprocessor expressions
DIAGNOSTIC(15200, Error, expectedTokenInPreprocessorExpression, "expected '{0}' in preprocessor expression")
DIAGNOSTIC(15201, Error, syntaxErrorInPreprocessorExpression, "syntax error in preprocessor expression")
DIAGNOSTIC(15202, Error, divideByZeroInPreprocessorExpression, "division by zero in preprocessor expression")
DIAGNOSTIC(15203, Error, expectedTokenInDefinedExpression, "expected '{0}' in 'defined' expression")
DIAGNOSTIC(15204, Warning, directiveExpectsExpression, "'{0}' directive requires an expression")
DIAGNOSTIC(15205, Warning, undefinedIdentifierInPreprocessorExpression, "undefined identifier '{0}' in preprocessor expression will evaluate to zero")

DIAGNOSTIC(-1, Note, seeOpeningToken, "see opening '{0}'")
// 153xx - #include
DIAGNOSTIC(15300, Error, includeFailed, "failed to find include file '{0}'")
DIAGNOSTIC(15301, Error, noUniqueIdentity, "`#include` handler didn't generate a unique identity for file '{0}'")

// 154xx - macro definition
DIAGNOSTIC(15400, Warning, macroRedefinition, "redefinition of macro '{0}'")
DIAGNOSTIC(15401, Warning, macroNotDefined, "macro '{0}' is not defined")
DIAGNOSTIC(15403, Error, expectedTokenInMacroParameters, "expected '{0}' in macro parameters")
DIAGNOSTIC(15404, Warning, builtinMacroRedefinition, "Redefinition of builtin macro '{0}'")

DIAGNOSTIC(15405, Error, tokenPasteAtStart, "'##' is not allowed at the start of a macro body")
DIAGNOSTIC(15406, Error, tokenPasteAtEnd, "'##' is not allowed at the end of a macro body")
DIAGNOSTIC(15407, Error, expectedMacroParameterAfterStringize, "'#' in macro body must be followed by the name of a macro parameter")
DIAGNOSTIC(15408, Error, duplicateMacroParameterName, "redefinition of macro parameter '{0}'")
DIAGNOSTIC(15409, Error, variadicMacroParameterMustBeLast, "a variadic macro parameter is only allowed at the end of the parameter list")

// 155xx - macro expansion
DIAGNOSTIC(15500, Warning, expectedTokenInMacroArguments, "expected '{0}' in macro invocation")
DIAGNOSTIC(15501, Error, wrongNumberOfArgumentsToMacro, "wrong number of arguments to macro (expected {0}, got {1})")
DIAGNOSTIC(15502, Error, errorParsingToMacroInvocationArgument, "error parsing macro '{0}' invocation argument to '{1}'")

DIAGNOSTIC(15503, Warning, invalidTokenPasteResult, "toking pasting with '##' resulted in the invalid token '{0}'")

// 156xx - pragmas
DIAGNOSTIC(15600, Error, expectedPragmaDirectiveName, "expected a name after '#pragma'")
DIAGNOSTIC(15601, Warning, unknownPragmaDirectiveIgnored, "ignoring unknown directive '#pragma {0}'")
DIAGNOSTIC(15602, Warning, pragmaOnceIgnored, "pragma once was ignored - this is typically because is not placed in an include")

// 159xx - user-defined error/warning
DIAGNOSTIC(15900, Error, userDefinedError, "#error: {0}")
DIAGNOSTIC(15901, Warning, userDefinedWarning, "#warning: {0}")

// 15999 waiting to be placed in the right range
DIAGNOSTIC(15999, Error, integerLiteralInvalidBase, "integer literal '{0}' has an invalid base, expected base '{1}'")
DIAGNOSTIC(15999, Error, integerLiteralOutOfRange, "integer literal '{0}' too large for type '{1}'")
DIAGNOSTIC(15999, Error, floatLiteralUnexpected, "float literal '{0}' unexpected parsing error")
DIAGNOSTIC(15999, Error, floatLiteralOutOfRange, "float literal '{0}' too large for type '{1}'")

//
// 2xxxx - Parsing
//
DIAGNOSTIC(20003, Error, unexpectedToken, "unexpected {0}")
DIAGNOSTIC(20001, Error, unexpectedTokenExpectedTokenType, "unexpected {0}, expected {1}")
DIAGNOSTIC(20001, Error, unexpectedTokenExpectedTokenName, "unexpected {0}, expected '{1}'")
DIAGNOSTIC(20002, Error, syntaxError, "syntax error.")

DIAGNOSTIC(20014, Error, duplicateKey, "Duplication of the key '{0}'")
DIAGNOSTIC(20015, Error, invalidParentsValue, "The value to inherit must be an array or an indetifier, but it's {0}.")
DIAGNOSTIC(20016, Error, invalidParentIndetifier, "The {0} is not supported as a parent indetifier.")
DIAGNOSTIC(20017, Error, invalidParentType, "The parent '{0}' is of type '{1}', but only objects can be used as parents.")
DIAGNOSTIC(20018, Error, undeclaredIdentifier, "Undeclared indetifier: '{0}'")

// 99999 - Internal compiler errors, and not-yet-classified diagnostics.
DIAGNOSTIC(20020, Error, divideByZero, "division by zero")
DIAGNOSTIC(20021, Error, inflixOnlyValidForType, "inflix operator {0} can only be applied to {1}")
DIAGNOSTIC(20021, Error, wrongTypeForUnary, "unary operator {0} can not be applied to {1}")

DIAGNOSTIC(99999, Internal, unimplemented, "unimplemented feature in Slang compiler: $0")
DIAGNOSTIC(99999, Internal, unexpected, "unexpected condition encountered in Slang compiler: {0}")
DIAGNOSTIC(99999, Internal, internalCompilerError, "Slang internal compiler error")
DIAGNOSTIC(99999, Error, compilationAborted, "Slang compilation aborted due to internal error")
DIAGNOSTIC(99999, Error, compilationAbortedDueToException, "Slang compilation aborted due to an exception of $0: $1")
DIAGNOSTIC(99999, Internal, serialDebugVerificationFailed, "Verification of serial debug information failed.")

DIAGNOSTIC(12345, Error, custom, "TODO Custom error: {0} : {1}")

#undef DIAGNOSTIC