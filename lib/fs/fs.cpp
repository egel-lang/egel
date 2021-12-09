#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <filesystem>

namespace fs = std::filesystem;
/**
 * Lift of C++ filesystem
 */

#define OS_STRING    "OS"

typedef std::vector<icu::UnicodeString> UnicodeStrings;

// convenience functions
VMObjectPtr path_to_object(const fs::path& p) {
    return VMObjectText(p.c_str()).clone();
}

VMObjectPtr paths_to_list(VM* vm, std::vector<fs::path> ss) {
    VMObjectPtrs oo;

    for (int n = ss.size() - 1; n >= 0; n--) {
        oo.push_back(path_to_object(ss[n]));
    }

    return vm->to_list(oo);
}

fs::path object_to_path(const VMObjectPtr& o) {
    auto str = VM_OBJECT_TEXT_VALUE(o);
    auto len = str.extract(0, 2048, nullptr, (uint32_t) 0); // XXX: I hate constants
    auto buffer = new char[len+1];
    str.extract(0, 2048, buffer, len+1);
    auto p = fs::path(buffer);
    delete[] buffer;
    return p;
}

VMObjectPtr error_to_object(const fs::filesystem_error& e) {
    auto s = e.what();
    return VMObjectText(s).clone();
}

//## OS::concat p0 p1 - concatenates two paths
class Concat: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Concat, OS_STRING, "concat");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                p0 += p1;

                return path_to_object(p0);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::concat_with p0 p1 - concatenates two paths with a directory separator
class ConcatWith: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, ConcatWith, OS_STRING, "concat_with");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                p0 /= p1;

                return path_to_object(p0);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::empty p - checks whether the path is empty
class Empty: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Empty, OS_STRING, "empty");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.empty();

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::has_root_path p - checks whether the path has a root path
class HasRootPath: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasRootPath, OS_STRING, "has_root_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_root_path();

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::has_root_name p - checks whether path has a root name
class HasRootName: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasRootName, OS_STRING, "has_root_name");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_root_name();

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::has_root_directory p - checks whether the path has a root directory
class HasRootDirectory: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasRootDirectory, OS_STRING, "has_root_directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_root_directory();

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::has_relative_path p - checks whether the path has a relative path
class HasRelativePath: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasRelativePath, OS_STRING, "has_relative_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_relative_path();

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::has_parent_path p - checks whether the path has a parent path
class HasParentPath: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasParentPath, OS_STRING, "has_parent_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_parent_path();

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::has_filename p - checks whether the path has a filename
class HasFilename: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasFilename, OS_STRING, "has_filename");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_filename();

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::has_stem p - checks whether the path has a stem
class HasStem: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasStem, OS_STRING, "has_stem");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_stem();

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::has_extension p - checks whether the path has an extension
class HasExtension: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasExtension, OS_STRING, "has_extension");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_extension();

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::is_absolute p - checks whether the path is absolute
class IsAbsolute: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsAbsolute, OS_STRING, "is_absolute");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.is_absolute();

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::is_relative p - checks whether the path is relative
class IsRelative: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsRelative, OS_STRING, "is_relative");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.is_relative();

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::root_name p - returns the root-name of the path, if present
class RootName: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RootName, OS_STRING, "root_name");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.root_name();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::root_directory p - returns the root directory of the path, if present
class RootDirectory: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RootDirectory, OS_STRING, "root_directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.root_directory();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::root_path p - returns the root path of the path, if present
class RootPath: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RootPath, OS_STRING, "root_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.root_path();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::relative_path p - returns path relative to the root path
class RelativePath: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RelativePath, OS_STRING, "relative_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.relative_path();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::parent_path p - returns the path of the parent path
class ParentPath: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, ParentPath, OS_STRING, "parent_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.parent_path();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::filename p - returns the filename path component
class Filename: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Filename, OS_STRING, "filename");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.filename();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::stem p - returns the stem path component
class Stem: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Stem, OS_STRING, "stem");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.stem();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::extension p - returns the file extension path component
class Extension: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Extension, OS_STRING, "extension");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.extension();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::absolute p - composes an absolute path
class Absolute: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Absolute, OS_STRING, "absolute");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = fs::absolute(p0);

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

/* Not in GCC yet
//# OS::canonical  - composes a canonical path
class Canonical: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Canonical, OS_STRING, "canonical");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = fs::canonical(p0);

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};
*/

/* Not in GCC yet
//# OS::weakly_canonical  - composes a weakly canonical path
class WeaklyCanonical: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, WeaklyCanonical, OS_STRING, "weakly_canonical");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = fs::weakly_canonical(p0);

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};
*/

/* Not in GCC yet
//# OS::relative - composes a relative path
class Relative: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Relative, OS_STRING, "relative");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                auto p2 = fs::relative(p0,p1);

                return path_to_object(p2);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};
*/

/* Not in GCC yet
//# OS::proximate - composes a relative path
class Proximate: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Proximate, OS_STRING, "proximate");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                auto p2 = fs::proximate(p0,p1);

                return path_to_object(p2);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};
*/

//## OS::copy src dst - copies files or directories
class Copy: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Copy, OS_STRING, "copy");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::copy(p0,p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::copy_file src dst - copies file contents
class CopyFile: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, CopyFile, OS_STRING, "copy_file");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::copy_file(p0,p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::copy_symlink src trg - copies a symbolic link
class CopySymlink: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, CopySymlink, OS_STRING, "copy_symlink");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::copy_symlink(p0,p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::create_directory p - creates new directory
class CreateDirectory: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, CreateDirectory, OS_STRING, "create_directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                fs::create_directory(p0);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::create_directories p - creates new directories
class CreateDirectories: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, CreateDirectories, OS_STRING, "create_directories");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                fs::create_directories(p0);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::create_hard_link p0 p1 - creates a hard link
class CreateHardLink: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, CreateHardLink, OS_STRING, "create_hard_link");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::create_hard_link(p0,p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::create_symlink p0 p1 - creates a symbolic link
class CreateSymlink: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, CreateSymlink, OS_STRING, "create_symlink");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::create_symlink(p0,p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::create_directory_symlink p0 p1 - creates a symbolic link
class CreateDirectorySymlink: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, CreateDirectorySymlink, OS_STRING, "create_directory_symlink");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::create_directory_symlink(p0,p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::current_path - returns the current working directory
class CurrentPath: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, CurrentPath, OS_STRING, "current_path");

    VMObjectPtr apply() const override {
        try {
            auto p = fs::current_path();

            return path_to_object(p);
        } catch (const fs::filesystem_error& e) {
            throw error_to_object(e);
        } 
    }
};

//## OS::set_current_path p - sets the current working directory
class SetCurrentPath: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, SetCurrentPath, OS_STRING, "set_current_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                fs::current_path(p0);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::exists p - checks whether path refers to existing file system object
class Exists: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Exists, OS_STRING, "exists");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::exists(p0);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::equivalent p0 p1 - checks whether two paths refer to the same file system object
class Equivalent: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Equivalent, OS_STRING, "equivalent");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                auto b = fs::equivalent(p0,p1);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::file_size p - returns the size of a file
class FileSize: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, FileSize, OS_STRING, "file_size");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = fs::file_size(p0);

                return machine()->create_integer(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::hard_link_count p - returns the number of hard links referring to the specific file
class HardLinkCount: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HardLinkCount, OS_STRING, "hard_link_count");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = fs::hard_link_count(p0);

                return machine()->create_integer(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//# OS::last_write_time - gets or sets the time of the last data modification XXX

//## OS::permissions p - get file access permissions
class Permissions: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Permissions, OS_STRING, "permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = (vm_int_t) fs::status(p0).permissions();

                return machine()->create_integer(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::replace_permissions p n - set file access permissions
class ReplacePermissions: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, ReplacePermissions, OS_STRING, "replace_permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_integer(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = machine()->get_integer(arg1);

                //fs::permissions(p0,(fs::perms) n1, fs::perm_options::replace);
                fs::permissions(p0,(fs::perms) n1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

/* Not in GCC yet
//# OS::add_permissions - add file access permissions
class AddPermissions: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, AddPermissions, OS_STRING, "add_permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_integer(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = machine()->get_integer(arg1);

                fs::permissions(p0,(fs::perms) n1, fs::perm_options::add);

                return machine->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//# OS::remove_permissions - remove file access permissions
class RemovePermissions: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, RemovePermissions, OS_STRING, "remove_permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_integer(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = machine()->get_integer(arg1);

                fs::permissions(p0,(fs::perms) n1, fs::perm_options::remove);

                return machine->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};
*/

//## OS::read_symlink p - obtains the target of a symbolic link
class ReadSymlink: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, ReadSymlink, OS_STRING, "read_symlink");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto p1 = fs::read_symlink(p0);

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::remove p - removes a file or empty directory
class Remove: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Remove, OS_STRING, "remove");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                fs::remove(p0);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::remove_all p - removes a file or directory and all its contents, recursively
class RemoveAll: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RemoveAll, OS_STRING, "remove_all");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                fs::remove_all(p0);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::rename p0 p1 - moves or renames a file or directory
class Rename: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Rename, OS_STRING, "rename");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::rename(p0,p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::resize_file p n - changes the size of a regular file by truncation or zero-fill
class ResizeFile: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, ResizeFile, OS_STRING, "resize_file");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_integer(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = machine()->get_integer(arg1);

                fs::resize_file(p0,n1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::space_free p - determines free space on the file system
class SpaceFree: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, SpaceFree, OS_STRING, "space_free");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = fs::space(p0).free;

                return machine()->create_integer(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::space_capacity p - determines capacity space on the file system
class SpaceCapacity: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, SpaceCapacity, OS_STRING, "space_capacity");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = fs::space(p0).capacity;

                return machine()->create_integer(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::space_available p - determines available space on the file system
class SpaceAvailable: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, SpaceAvailable, OS_STRING, "space_available");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = fs::space(p0).available;

                return machine()->create_integer(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//# OS::status - determines file attributes XXX
//# OS::symlink_status - determines file attributes, checking the symlink target XXX

//## OS::temp_directory_path - returns a directory suitable for temporary files
class TempDirectoryPath: public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, TempDirectoryPath, OS_STRING, "temp_directory_path");

    VMObjectPtr apply() const override {
        try {
            auto p = fs::temp_directory_path();

            return path_to_object(p);
        } catch (const fs::filesystem_error& e) {
            throw error_to_object(e);
        } 
    }
};

//## OS::is_block_file p - checks whether the given path refers to block device
class IsBlockFile: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsBlockFile, OS_STRING, "is_block_file");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_block_file(p0);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::is_character_file p - checks whether the given path refers to a character device
class IsCharacterFile: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsCharacterFile, OS_STRING, "is_character_file");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_character_file(p0);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::is_directory p - checks whether the given path refers to a directory
class IsDirectory: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsDirectory, OS_STRING, "is_directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_directory(p0);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::is_empty p - checks whether the given path refers to an empty file or directory
class IsEmpty: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsEmpty, OS_STRING, "is_empty");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_empty(p0);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::is_fifo p - checks whether the given path refers to a named pipe
class IsFifo: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsFifo, OS_STRING, "is_fifo");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_fifo(p0);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::is_other p - checks whether the argument refers to an other file
class IsOther: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsOther, OS_STRING, "is_other");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_other(p0);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::is_regular_file p - checks whether the argument refers to a regular file
class IsRegularFile: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsRegularFile, OS_STRING, "is_regular_file");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_regular_file(p0);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};

//## OS::is_socket p - checks whether the argument refers to a named IPC socket
class IsSocket: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsSocket, OS_STRING, "is_socket");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_socket(p0);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::is_symlink p - checks whether the argument refers to a symbolic link
class IsSymlink: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsSymlink, OS_STRING, "is_symlink");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_symlink(p0);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::directory p - lists the content of a directory
class Directory: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Directory, OS_STRING, "directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                std::vector<fs::path> ff;
                for(auto& f: fs::directory_iterator(p0)) {
                    ff.push_back(f);
                }

                return paths_to_list(machine(), ff);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            THROW_BADARGS;
        }
    }
};


//## OS::status_known - checks whether file status is known XXX

extern "C" std::vector<icu::UnicodeString> egel_imports() {
    return std::vector<icu::UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(Concat(vm).clone());
    oo.push_back(ConcatWith(vm).clone());
    oo.push_back(Empty(vm).clone());
    oo.push_back(HasRootPath(vm).clone());
    oo.push_back(HasRootName(vm).clone());
    oo.push_back(HasRootDirectory(vm).clone());
    oo.push_back(HasRelativePath(vm).clone());
    oo.push_back(HasParentPath(vm).clone());
    oo.push_back(HasFilename(vm).clone());
    oo.push_back(HasStem(vm).clone());
    oo.push_back(HasExtension(vm).clone());
    oo.push_back(IsAbsolute(vm).clone());
    oo.push_back(IsRelative(vm).clone());
    oo.push_back(RootName(vm).clone());
    oo.push_back(RootDirectory(vm).clone());
    oo.push_back(RootPath(vm).clone());
    oo.push_back(RelativePath(vm).clone());
    oo.push_back(ParentPath(vm).clone());
    oo.push_back(Filename(vm).clone());
    oo.push_back(Stem(vm).clone());
    oo.push_back(Extension(vm).clone());
    oo.push_back(Absolute(vm).clone());
    //oo.push_back(Canonical(vm).clone());
    //oo.push_back(WeaklyCanonical(vm).clone());
    //oo.push_back(Relative(vm).clone());
    //oo.push_back(Proximate(vm).clone());
    oo.push_back(Copy(vm).clone());
    oo.push_back(CopyFile(vm).clone());
    oo.push_back(CopySymlink(vm).clone());
    oo.push_back(CreateDirectory(vm).clone());
    oo.push_back(CreateDirectories(vm).clone());
    oo.push_back(CreateHardLink(vm).clone());
    oo.push_back(CreateSymlink(vm).clone());
    oo.push_back(CreateDirectorySymlink(vm).clone());
    oo.push_back(CurrentPath(vm).clone());
    oo.push_back(SetCurrentPath(vm).clone());
    oo.push_back(Exists(vm).clone());
    oo.push_back(Equivalent(vm).clone());
    oo.push_back(FileSize(vm).clone());
    oo.push_back(HardLinkCount(vm).clone());
    oo.push_back(Permissions(vm).clone());
    oo.push_back(ReplacePermissions(vm).clone());
    //oo.push_back(AddPermissions(vm).clone());
    //oo.push_back(RemovePermissions(vm).clone());
    oo.push_back(ReadSymlink(vm).clone());
    oo.push_back(Remove(vm).clone());
    oo.push_back(RemoveAll(vm).clone());
    oo.push_back(Rename(vm).clone());
    oo.push_back(ResizeFile(vm).clone());
    oo.push_back(SpaceFree(vm).clone());
    oo.push_back(SpaceCapacity(vm).clone());
    oo.push_back(SpaceAvailable(vm).clone());
    oo.push_back(TempDirectoryPath(vm).clone());
    oo.push_back(IsBlockFile(vm).clone());
    oo.push_back(IsCharacterFile(vm).clone());
    oo.push_back(IsDirectory(vm).clone());
    oo.push_back(IsEmpty(vm).clone());
    oo.push_back(IsFifo(vm).clone());
    oo.push_back(IsOther(vm).clone());
    oo.push_back(IsRegularFile(vm).clone());
    oo.push_back(IsSocket(vm).clone());
    oo.push_back(IsSymlink(vm).clone());
    oo.push_back(Directory(vm).clone());

    return oo;
}
