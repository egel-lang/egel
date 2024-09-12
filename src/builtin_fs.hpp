#include <stdlib.h>

#include <filesystem>

#include "runtime.hpp"

namespace fs = std::filesystem;

/**
 * Lift of C++ filesystem
 */

namespace egel {
#define OS_STRING "OS"

typedef std::vector<icu::UnicodeString> UnicodeStrings;

// convenience functions
VMObjectPtr path_to_object(const fs::path& p) {
    return VMObjectText::create(p.c_str());
}

VMObjectPtr paths_to_list(VM* vm, std::vector<fs::path> ss) {
    VMObjectPtrs oo;

    for (int n = ss.size() - 1; n >= 0; n--) {
        oo.push_back(path_to_object(ss[n]));
    }

    return vm->to_list(oo);
}

fs::path object_to_path(const VMObjectPtr& o) {
    auto str = VMObjectText::value(o);
    auto len =
        str.extract(0, 2048, nullptr, (uint32_t)0);  // XXX: I hate constants
    auto buffer = new char[len + 1];
    str.extract(0, 2048, buffer, len + 1);
    auto p = fs::path(buffer);
    delete[] buffer;
    return p;
}

VMObjectPtr error_to_object(const fs::filesystem_error& e) {
    auto s = e.what();
    return VMObjectText::create(s);
}

class Concat : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Concat, OS_STRING, "concat");
    DOCSTRING("OS::concat p0 p1 - concatenates two paths");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
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
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class ConcatWith : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, ConcatWith, OS_STRING, "concat_with");
    DOCSTRING(
        "OS::concat_with p0 p1 - concatenates two paths with a directory "
        "separator");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
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
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class Empty : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Empty, OS_STRING, "empty");
    DOCSTRING("OS::empty p - checks whether the path is empty");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class HasRootPath : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasRootPath, OS_STRING, "has_root_path");
    DOCSTRING("OS::has_root_path p - checks whether the path has a root path");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class HasRootName : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasRootName, OS_STRING, "has_root_name");
    DOCSTRING("OS::has_root_name p - checks whether path has a root name");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class HasRootDirectory : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasRootDirectory, OS_STRING,
                     "has_root_directory");
    DOCSTRING(
        "OS::has_root_directory p - checks whether the path has a root "
        "directory");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class HasRelativePath : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasRelativePath, OS_STRING,
                     "has_relative_path");
    DOCSTRING(
        "OS::has_relative_path p - checks whether the path has a relative "
        "path");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class HasParentPath : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasParentPath, OS_STRING, "has_parent_path");
    DOCSTRING(
        "OS::has_parent_path p - checks whether the path has a parent path");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class HasFilename : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasFilename, OS_STRING, "has_filename");
    DOCSTRING("OS::has_filename p - checks whether the path has a filename");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class HasStem : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasStem, OS_STRING, "has_stem");
    DOCSTRING("OS::has_stem p - checks whether the path has a stem");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class HasExtension : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HasExtension, OS_STRING, "has_extension");
    DOCSTRING("OS::has_extension p - checks whether the path has an extension");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsAbsolute : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsAbsolute, OS_STRING, "is_absolute");
    DOCSTRING("OS::is_absolute p - checks whether the path is absolute");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsRelative : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsRelative, OS_STRING, "is_relative");
    DOCSTRING("OS::is_relative p - checks whether the path is relative");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class RootName : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RootName, OS_STRING, "root_name");
    DOCSTRING(
        "OS::root_name p - returns the root-name of the path, if present");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class RootDirectory : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RootDirectory, OS_STRING, "root_directory");
    DOCSTRING(
        "OS::root_directory p - returns the root directory of the path, if "
        "present");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class RootPath : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RootPath, OS_STRING, "root_path");
    DOCSTRING(
        "OS::root_path p - returns the root path of the path, if present");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class RelativePath : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RelativePath, OS_STRING, "relative_path");
    DOCSTRING("OS::relative_path p - returns path relative to the root path");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class ParentPath : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, ParentPath, OS_STRING, "parent_path");
    DOCSTRING("OS::parent_path p - returns the path of the parent path");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Filename : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Filename, OS_STRING, "filename");
    DOCSTRING("OS::filename p - returns the filename path component");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Stem : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Stem, OS_STRING, "stem");
    DOCSTRING("OS::stem p - returns the stem path component");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Extension : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Extension, OS_STRING, "extension");
    DOCSTRING("OS::extension p - returns the file extension path component");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Absolute : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Absolute, OS_STRING, "absolute");
    DOCSTRING("OS::absolute p - composes an absolute path");

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
            throw machine()->bad_args(this, arg0);
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
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};
*/

/* Not in GCC yet
//# OS::weakly_canonical  - composes a weakly canonical path
class WeaklyCanonical: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, WeaklyCanonical, OS_STRING,
"weakly_canonical");

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
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};
*/

/* Not in GCC yet
//# OS::relative - composes a relative path
class Relative: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Relative, OS_STRING, "relative");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const
override { if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) { try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                auto p2 = fs::relative(p0,p1);

                return path_to_object(p2);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};
*/

/* Not in GCC yet
//# OS::proximate - composes a relative path
class Proximate: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Proximate, OS_STRING, "proximate");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const
override { if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) { try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                auto p2 = fs::proximate(p0,p1);

                return path_to_object(p2);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};
*/

class Copy : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Copy, OS_STRING, "copy");
    DOCSTRING("OS::copy src dst - copies files or directories");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::copy(p0, p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class CopyFile : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, CopyFile, OS_STRING, "copy_file");
    DOCSTRING("OS::copy_file src dst - copies file contents");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::copy_file(p0, p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class CopySymlink : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, CopySymlink, OS_STRING, "copy_symlink");
    DOCSTRING("OS::copy_symlink src trg - copies a symbolic link");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::copy_symlink(p0, p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class CreateDirectory : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, CreateDirectory, OS_STRING,
                     "create_directory");
    DOCSTRING("OS::create_directory p - creates new directory");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class CreateDirectories : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, CreateDirectories, OS_STRING,
                     "create_directories");
    DOCSTRING("OS::create_directories p - creates new directories");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class CreateHardLink : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, CreateHardLink, OS_STRING, "create_hard_link");
    DOCSTRING("OS::create_hard_link p0 p1 - creates a hard link");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::create_hard_link(p0, p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class CreateSymlink : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, CreateSymlink, OS_STRING, "create_symlink");
    DOCSTRING("OS::create_symlink p0 p1 - creates a symbolic link");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::create_symlink(p0, p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class CreateDirectorySymlink : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, CreateDirectorySymlink, OS_STRING,
                    "create_directory_symlink");
    DOCSTRING("OS::create_directory_symlink p0 p1 - creates a symbolic link");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::create_directory_symlink(p0, p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class CurrentPath : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, CurrentPath, OS_STRING, "current_path");
    DOCSTRING("OS::current_path - returns the current working directory");

    VMObjectPtr apply() const override {
        try {
            auto p = fs::current_path();

            return path_to_object(p);
        } catch (const fs::filesystem_error& e) {
            throw error_to_object(e);
        }
    }
};

class SetCurrentPath : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, SetCurrentPath, OS_STRING, "set_current_path");
    DOCSTRING("OS::set_current_path p - sets the current working directory");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Exists : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Exists, OS_STRING, "exists");
    DOCSTRING(
        "OS::exists p - checks whether path refers to existing file system "
        "object");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

//  system object
class Equivalent : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Equivalent, OS_STRING, "equivalent");
    DOCSTRING(
        "OS::equivalent p0 p1 - checks whether two paths refer to the same "
        "file");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                auto b = fs::equivalent(p0, p1);

                return machine()->create_bool(b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class FileSize : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, FileSize, OS_STRING, "file_size");
    DOCSTRING("OS::file_size p - returns the size of a file");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class HardLinkCount : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, HardLinkCount, OS_STRING, "hard_link_count");
    DOCSTRING("OS::hard_link_count p - returns the number of hard links ");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

// # OS::last_write_time - gets or sets the time of the last data modification
//  XXX

class Permissions : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Permissions, OS_STRING, "permissions");
    DOCSTRING("OS::permissions p - get file access permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = (vm_int_t)fs::status(p0).permissions();

                return machine()->create_integer(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class ReplacePermissions : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, ReplacePermissions, OS_STRING,
                    "replace_permissions");
    DOCSTRING("OS::replace_permissions p n - set file access permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_integer(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = machine()->get_integer(arg1);

                // fs::permissions(p0,(fs::perms) n1,
                // fs::perm_options::replace);
                fs::permissions(p0, (fs::perms)n1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

/* Not in GCC yet
//# OS::add_permissions - add file access permissions
class AddPermissions: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, AddPermissions, OS_STRING, "add_permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const
override { if ((machine()->is_text(arg0)) && (machine()->is_integer(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = machine()->get_integer(arg1);

                fs::permissions(p0,(fs::perms) n1, fs::perm_options::add);

                return machine->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

//# OS::remove_permissions - remove file access permissions
class RemovePermissions: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, RemovePermissions, OS_STRING,
"remove_permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const
override { if ((machine()->is_text(arg0)) && (machine()->is_integer(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = machine()->get_integer(arg1);

                fs::permissions(p0,(fs::perms) n1, fs::perm_options::remove);

                return machine->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};
*/

class ReadSymlink : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, ReadSymlink, OS_STRING, "read_symlink");
    DOCSTRING("OS::read_symlink p - obtains the target of a symbolic link");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class RemoveFile : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RemoveFile, OS_STRING, "remove_file");
    DOCSTRING("OS::remove_file p - removes a file or empty directory");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class RemoveAll : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RemoveAll, OS_STRING, "remove_all");
    DOCSTRING(
        "OS::remove_all p - removes a file or directory and all its contents");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Rename : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, Rename, OS_STRING, "rename");
    DOCSTRING("OS::rename p0 p1 - moves or renames a file or directory");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_text(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::rename(p0, p1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class ResizeFile : public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, ResizeFile, OS_STRING, "resize_file");
    DOCSTRING("OS::resize_file p n - changes the size of a regular file ");

    VMObjectPtr apply(const VMObjectPtr& arg0,
                      const VMObjectPtr& arg1) const override {
        if ((machine()->is_text(arg0)) && (machine()->is_integer(arg1))) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = machine()->get_integer(arg1);

                fs::resize_file(p0, n1);

                return machine()->create_none();
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class SpaceFree : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, SpaceFree, OS_STRING, "space_free");
    DOCSTRING("OS::space_free p - determines free space on the file system");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class SpaceCapacity : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, SpaceCapacity, OS_STRING, "space_capacity");
    DOCSTRING(
        "OS::space_capacity p - determines capacity space on the file system");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class SpaceAvailable : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, SpaceAvailable, OS_STRING, "space_available");
    DOCSTRING(
        "OS::space_available p - determines available space on the file "
        "system");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

// # OS::status - determines file attributes XXX
// # OS::symlink_status - determines file attributes, checking the symlink
// target
//  XXX

class TempDirectoryPath : public Medadic {
public:
    MEDADIC_PREAMBLE(VM_SUB_EGO, TempDirectoryPath, OS_STRING,
                     "temp_directory_path");
    DOCSTRING(
        "OS::temp_directory_path - returns a directory suitable for temporary "
        "files");

    VMObjectPtr apply() const override {
        try {
            auto p = fs::temp_directory_path();

            return path_to_object(p);
        } catch (const fs::filesystem_error& e) {
            throw error_to_object(e);
        }
    }
};

class IsBlockFile : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsBlockFile, OS_STRING, "is_block_file");
    DOCSTRING(
        "OS::is_block_file p - checks whether the given path refers to block "
        "device");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsCharacterFile : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsCharacterFile, OS_STRING,
                     "is_character_file");
    DOCSTRING(
        "OS::is_character_file p - the given path refers to a character "
        "device");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsDirectory : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsDirectory, OS_STRING, "is_directory");
    DOCSTRING(
        "OS::is_directory p - checks whether the given path refers to a "
        "directory");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

//  directory
class IsEmptyFile : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsEmptyFile, OS_STRING, "is_empty_file");
    DOCSTRING(
        "OS::is_empty_file p - checks whether the given path refers to an "
        "empty file");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsFifo : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsFifo, OS_STRING, "is_fifo");
    DOCSTRING(
        "OS::is_fifo p - checks whether the given path refers to a named pipe");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsOther : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsOther, OS_STRING, "is_other");
    DOCSTRING(
        "OS::is_other p - checks whether the argument refers to an other file");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsRegularFile : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsRegularFile, OS_STRING, "is_regular_file");
    DOCSTRING("OS::is_regular_file p - the argument refers to a regular file");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsSocket : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsSocket, OS_STRING, "is_socket");
    DOCSTRING(
        "OS::is_socket p - checks whether the argument refers to a named IPC "
        "socket");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class IsSymlink : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, IsSymlink, OS_STRING, "is_symlink");
    DOCSTRING(
        "OS::is_symlink p - checks whether the argument refers to a symbolic "
        "link");

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
            throw machine()->bad_args(this, arg0);
        }
    }
};

class Directory : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, Directory, OS_STRING, "directory");
    DOCSTRING("OS::directory p - lists the content of a directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            try {
                auto p0 = object_to_path(arg0);

                std::vector<fs::path> ff;
                for (auto& f : fs::directory_iterator(p0)) {
                    ff.push_back(f);
                }

                return paths_to_list(machine(), ff);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            }
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

// DOCSTRING("OS::status_known - checks whether file status is known XXX");
class FSModule : public CModule {
public:
    icu::UnicodeString name() const override {
        return "fs";
    }

    icu::UnicodeString docstring() const override {
        return "The 'fs' module defines file system inspection and "
               "modification combinators.";
    }

    std::vector<VMObjectPtr> exports(VM* vm) override {
        std::vector<VMObjectPtr> oo;

        oo.push_back(Concat::create(vm));
        oo.push_back(ConcatWith::create(vm));
        oo.push_back(Empty::create(vm));
        oo.push_back(HasRootPath::create(vm));
        oo.push_back(HasRootName::create(vm));
        oo.push_back(HasRootDirectory::create(vm));
        oo.push_back(HasRelativePath::create(vm));
        oo.push_back(HasParentPath::create(vm));
        oo.push_back(HasFilename::create(vm));
        oo.push_back(HasStem::create(vm));
        oo.push_back(HasExtension::create(vm));
        oo.push_back(IsAbsolute::create(vm));
        oo.push_back(IsRelative::create(vm));
        oo.push_back(RootName::create(vm));
        oo.push_back(RootDirectory::create(vm));
        oo.push_back(RootPath::create(vm));
        oo.push_back(RelativePath::create(vm));
        oo.push_back(ParentPath::create(vm));
        oo.push_back(Filename::create(vm));
        oo.push_back(Stem::create(vm));
        oo.push_back(Extension::create(vm));
        oo.push_back(Absolute::create(vm));
        // oo.push_back(Canonical::create(vm));
        // oo.push_back(WeaklyCanonical::create(vm));
        // oo.push_back(Relative::create(vm));
        // oo.push_back(Proximate::create(vm));
        oo.push_back(Copy::create(vm));
        oo.push_back(CopyFile::create(vm));
        oo.push_back(CopySymlink::create(vm));
        oo.push_back(CreateDirectory::create(vm));
        oo.push_back(CreateDirectories::create(vm));
        oo.push_back(CreateHardLink::create(vm));
        oo.push_back(CreateSymlink::create(vm));
        oo.push_back(CreateDirectorySymlink::create(vm));
        oo.push_back(CurrentPath::create(vm));
        oo.push_back(SetCurrentPath::create(vm));
        oo.push_back(Exists::create(vm));
        oo.push_back(Equivalent::create(vm));
        oo.push_back(FileSize::create(vm));
        oo.push_back(HardLinkCount::create(vm));
        oo.push_back(Permissions::create(vm));
        oo.push_back(ReplacePermissions::create(vm));
        // oo.push_back(AddPermissions::create(vm));
        // oo.push_back(RemovePermissions::create(vm));
        oo.push_back(ReadSymlink::create(vm));
        oo.push_back(RemoveFile::create(vm));
        oo.push_back(RemoveAll::create(vm));
        oo.push_back(Rename::create(vm));
        oo.push_back(ResizeFile::create(vm));
        oo.push_back(SpaceFree::create(vm));
        oo.push_back(SpaceCapacity::create(vm));
        oo.push_back(SpaceAvailable::create(vm));
        oo.push_back(TempDirectoryPath::create(vm));
        oo.push_back(IsBlockFile::create(vm));
        oo.push_back(IsCharacterFile::create(vm));
        oo.push_back(IsDirectory::create(vm));
        oo.push_back(IsEmptyFile::create(vm));
        oo.push_back(IsFifo::create(vm));
        oo.push_back(IsOther::create(vm));
        oo.push_back(IsRegularFile::create(vm));
        oo.push_back(IsSocket::create(vm));
        oo.push_back(IsSymlink::create(vm));
        oo.push_back(Directory::create(vm));

        return oo;
    }
};

}  // namespace egel
