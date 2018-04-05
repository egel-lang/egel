#include "../../src/runtime.hpp"

#include <stdlib.h>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
/**
 * Start of a simplistic Regex library lifting most of libicu.
 */

#define FS_STRING    "FS"

typedef std::vector<UnicodeString> UnicodeStrings;

// convenience functions
VMObjectPtr path_to_object(const fs::path& p) {
    return VMObjectText(p.c_str()).clone();
}

VMObjectPtr paths_to_list(VM* vm, std::vector<fs::path> ss) {
    auto _nil = vm->get_data_string("System", "nil");

    auto _cons = vm->get_data_string("System", "cons");

    VMObjectPtr result = _nil;

    for (int n = ss.size() - 1; n >= 0; n--) {
        VMObjectPtrs vv;
        vv.push_back(_cons);
        vv.push_back(path_to_object(ss[n]));
        vv.push_back(result);

        result = VMObjectArray(vv).clone();
    }

    return result;
}

fs::path object_to_path(const VMObjectPtr& o) {
    auto str = VM_OBJECT_TEXT_VALUE(o);
    auto len = str.extract(0, 2048, nullptr, (uint32_t) 0); // XXX: I hate constants
    auto buffer = new char[len+1];
    str.extract(0, 2048, buffer, len+1);
    auto p = fs::path(buffer);
    delete buffer;
    return p;
}

VMObjectPtr error_to_object(const fs::filesystem_error& e) {
    auto s = e.what();
    return VMObjectText(s).clone();
}

// XXX: move this to the VM once
VMObjectPtr bool_to_object(VM* vm, const bool& b) {
    if (b) {
        return vm->get_data_string("System", "true");
    } else {
        return vm->get_data_string("System", "false");
    }
}

VMObjectPtr int_to_object(const vm_int_t& n) {
    return VMObjectInteger(n).clone();
}

VMObjectPtr nop(VM* vm) {
    return vm->get_data_string("System", "nop");
}

// FS.empty
class Empty: public Monadic {
public:
    MONADIC_PREAMBLE(Empty, FS_STRING, "empty");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.empty();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.has_root_path
class HasRootPath: public Monadic {
public:
    MONADIC_PREAMBLE(HasRootPath, FS_STRING, "has_root_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_root_path();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.has_root_name
class HasRootName: public Monadic {
public:
    MONADIC_PREAMBLE(HasRootName, FS_STRING, "has_root_name");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_root_name();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.has_root_directory
class HasRootDirectory: public Monadic {
public:
    MONADIC_PREAMBLE(HasRootDirectory, FS_STRING, "has_root_directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_root_directory();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.has_relative_path
class HasRelativePath: public Monadic {
public:
    MONADIC_PREAMBLE(HasRelativePath, FS_STRING, "has_relative_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_relative_path();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.has_parent_path
class HasParentPath: public Monadic {
public:
    MONADIC_PREAMBLE(HasParentPath, FS_STRING, "has_parent_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_parent_path();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.has_filename
class HasFilename: public Monadic {
public:
    MONADIC_PREAMBLE(HasFilename, FS_STRING, "has_filename");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_filename();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.has_stem
class HasStem: public Monadic {
public:
    MONADIC_PREAMBLE(HasStem, FS_STRING, "has_stem");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_stem();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.has_extension
class HasExtension: public Monadic {
public:
    MONADIC_PREAMBLE(HasExtension, FS_STRING, "has_extension");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.has_extension();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.is_absolute
class IsAbsolute: public Monadic {
public:
    MONADIC_PREAMBLE(IsAbsolute, FS_STRING, "is_absolute");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.is_absolute();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.is_relative
class IsRelative: public Monadic {
public:
    MONADIC_PREAMBLE(IsRelative, FS_STRING, "is_relative");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto b = p0.is_relative();

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.root_name - returns the root-name of the path, if present
class RootName: public Monadic {
public:
    MONADIC_PREAMBLE(RootName, FS_STRING, "root_name");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.root_name();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.root_directory - returns the root directory of the path, if present
class RootDirectory: public Monadic {
public:
    MONADIC_PREAMBLE(RootDirectory, FS_STRING, "root_directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.root_directory();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.root_path - returns the root path of the path, if present
class RootPath: public Monadic {
public:
    MONADIC_PREAMBLE(RootPath, FS_STRING, "root_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.root_path();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.relative_path - returns path relative to the root path
class RelativePath: public Monadic {
public:
    MONADIC_PREAMBLE(RelativePath, FS_STRING, "relative_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.relative_path();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.parent_path - returns the path of the parent path
class ParentPath: public Monadic {
public:
    MONADIC_PREAMBLE(ParentPath, FS_STRING, "parent_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.parent_path();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.filename - returns the filename path component
class Filename: public Monadic {
public:
    MONADIC_PREAMBLE(Filename, FS_STRING, "filename");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.filename();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.stem - returns the stem path component
class Stem: public Monadic {
public:
    MONADIC_PREAMBLE(Stem, FS_STRING, "stem");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.stem();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.extension - returns the file extension path component
class Extension: public Monadic {
public:
    MONADIC_PREAMBLE(Extension, FS_STRING, "extension");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = p0.extension();

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.absolute composes an absolute path
class Absolute: public Monadic {
public:
    MONADIC_PREAMBLE(Absolute, FS_STRING, "absolute");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = fs::absolute(p0);

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

/* Not in GCC yet
// FS.canonical  - composes a canonical path
class Canonical: public Monadic {
public:
    MONADIC_PREAMBLE(Canonical, FS_STRING, "canonical");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = fs::canonical(p0);

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};
*/

/* Not in GCC yet
// FS.weakly_canonical  - composes a weakly canonical path
class WeaklyCanonical: public Monadic {
public:
    MONADIC_PREAMBLE(WeaklyCanonical, FS_STRING, "weakly_canonical");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = fs::weakly_canonical(p0);

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};
*/

/* Not in GCC yet
// FS.relative - composes a relative path
class Relative: public Dyadic {
public:
    DYADIC_PREAMBLE(Relative, FS_STRING, "relative");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                auto p2 = fs::relative(p0,p1);

                return path_to_object(p2);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};
*/

/* Not in GCC yet
// FS.proximate - composes a relative path
class Proximate: public Dyadic {
public:
    DYADIC_PREAMBLE(Proximate, FS_STRING, "proximate");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                auto p2 = fs::proximate(p0,p1);

                return path_to_object(p2);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};
*/

// FS.copy - copies files or directories
class Copy: public Dyadic {
public:
    DYADIC_PREAMBLE(Copy, FS_STRING, "copy");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::copy(p0,p1);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.copy_file - copies file contents
class CopyFile: public Dyadic {
public:
    DYADIC_PREAMBLE(CopyFile, FS_STRING, "copy_file");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::copy_file(p0,p1);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.copy_symlink - copies a symbolic link
class CopySymlink: public Dyadic {
public:
    DYADIC_PREAMBLE(CopySymlink, FS_STRING, "copy_symlink");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::copy_symlink(p0,p1);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.create_directory - creates new directory
class CreateDirectory: public Monadic {
public:
    MONADIC_PREAMBLE(CreateDirectory, FS_STRING, "create_directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                fs::create_directory(p0);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.create_directories - creates new directory
class CreateDirectories: public Monadic {
public:
    MONADIC_PREAMBLE(CreateDirectories, FS_STRING, "create_directories");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                fs::create_directories(p0);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.create_hard_link - creates a hard link
class CreateHardLink: public Dyadic {
public:
    DYADIC_PREAMBLE(CreateHardLink, FS_STRING, "create_hard_link");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::create_hard_link(p0,p1);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.create_symlink - creates a symbolic link
class CreateSymlink: public Dyadic {
public:
    DYADIC_PREAMBLE(CreateSymlink, FS_STRING, "create_symlink");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::create_symlink(p0,p1);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.create_directory_symlink - creates a symbolic link
class CreateDirectorySymlink: public Dyadic {
public:
    DYADIC_PREAMBLE(CreateDirectorySymlink, FS_STRING, "create_directory_symlink");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::create_directory_symlink(p0,p1);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.current_path - returns the current working directory
class CurrentPath: public Medadic {
public:
    MEDADIC_PREAMBLE(CurrentPath, FS_STRING, "current_path");

    VMObjectPtr apply() const override {
        try {
            auto p = fs::current_path();

            return path_to_object(p);
        } catch (const fs::filesystem_error& e) {
            throw error_to_object(e);
        } 
    }
};

// FS.set_current_path - sets the current working directory
class SetCurrentPath: public Monadic {
public:
    MONADIC_PREAMBLE(SetCurrentPath, FS_STRING, "set_current_path");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                fs::current_path(p0);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.exists - checks whether path refers to existing file system object
class Exists: public Monadic {
public:
    MONADIC_PREAMBLE(Exists, FS_STRING, "exists");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::exists(p0);

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.equivalent - checks whether two paths refer to the same file system object
class Equivalent: public Dyadic {
public:
    DYADIC_PREAMBLE(Equivalent, FS_STRING, "equivalent");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                auto b = fs::equivalent(p0,p1);

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.file_size - returns the size of a file
class FileSize: public Monadic {
public:
    MONADIC_PREAMBLE(FileSize, FS_STRING, "file_size");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = fs::file_size(p0);

                return int_to_object(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.hard_link_count - returns the number of hard links referring to the specific file
class HardLinkCount: public Monadic {
public:
    MONADIC_PREAMBLE(HardLinkCount, FS_STRING, "hard_link_count");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = fs::hard_link_count(p0);

                return int_to_object(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.last_write_time - gets or sets the time of the last data modification XXX

// FS.permissions - get file access permissions
class Permissions: public Monadic {
public:
    MONADIC_PREAMBLE(Permissions, FS_STRING, "permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = (vm_int_t) fs::status(p0).permissions();

                return int_to_object(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.replace_permissions - set file access permissions
class ReplacePermissions: public Dyadic {
public:
    DYADIC_PREAMBLE(ReplacePermissions, FS_STRING, "replace_permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_INTEGER)) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = VM_OBJECT_INTEGER_VALUE(arg1);

                //fs::permissions(p0,(fs::perms) n1, fs::perm_options::replace);
                fs::permissions(p0,(fs::perms) n1);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

/* Not in GCC yet
// FS.add_permissions - add file access permissions
class AddPermissions: public Dyadic {
public:
    DYADIC_PREAMBLE(AddPermissions, FS_STRING, "add_permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_INTEGER)) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = VM_OBJECT_INTEGER_VALUE(arg1);

                fs::permissions(p0,(fs::perms) n1, fs::perm_options::add);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.remove_permissions - remove file access permissions
class RemovePermissions: public Dyadic {
public:
    DYADIC_PREAMBLE(RemovePermissions, FS_STRING, "remove_permissions");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_INTEGER)) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = VM_OBJECT_INTEGER_VALUE(arg1);

                fs::permissions(p0,(fs::perms) n1, fs::perm_options::remove);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};
*/

// FS.read_symlink - obtains the target of a symbolic link
class ReadSymlink: public Monadic {
public:
    MONADIC_PREAMBLE(ReadSymlink, FS_STRING, "read_symlink");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto p1 = fs::read_symlink(p0);

                return path_to_object(p1);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.remove - removes a file or empty directory
class Remove: public Monadic {
public:
    MONADIC_PREAMBLE(Remove, FS_STRING, "remove");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                fs::remove(p0);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.remove_all - removes a file or directory and all its contents, recursively
class RemoveAll: public Monadic {
public:
    MONADIC_PREAMBLE(RemoveAll, FS_STRING, "remove_all");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                fs::remove_all(p0);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.rename - moves or renames a file or directory
class Rename: public Dyadic {
public:
    DYADIC_PREAMBLE(Rename, FS_STRING, "rename");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_TEXT)) {
            try {
                auto p0 = object_to_path(arg0);
                auto p1 = object_to_path(arg1);

                fs::rename(p0,p1);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.resize_file - changes the size of a regular file by truncation or zero-fill
class ResizeFile: public Dyadic {
public:
    DYADIC_PREAMBLE(ResizeFile, FS_STRING, "resize_file");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        if ((arg0->tag() == VM_OBJECT_TEXT) && (arg1->tag() == VM_OBJECT_INTEGER)) {
            try {
                auto p0 = object_to_path(arg0);
                auto n1 = VM_OBJECT_INTEGER_VALUE(arg1);

                fs::resize_file(p0,n1);

                return nop(machine());
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.space_free - determines free space on the file system
class SpaceFree: public Monadic {
public:
    MONADIC_PREAMBLE(SpaceFree, FS_STRING, "space_free");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = fs::space(p0).free;

                return int_to_object(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.space_capacity - determines capacity space on the file system
class SpaceCapacity: public Monadic {
public:
    MONADIC_PREAMBLE(SpaceCapacity, FS_STRING, "space_capacity");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = fs::space(p0).capacity;

                return int_to_object(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.space_available - determines available space on the file system
class SpaceAvailable: public Monadic {
public:
    MONADIC_PREAMBLE(SpaceAvailable, FS_STRING, "space_available");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto n = fs::space(p0).available;

                return int_to_object(n);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.status - determines file attributes XXX
// FS.symlink_status - determines file attributes, checking the symlink target XXX

// FS.temp_directory_path - returns a directory suitable for temporary files
class TempDirectoryPath: public Medadic {
public:
    MEDADIC_PREAMBLE(TempDirectoryPath, FS_STRING, "temp_directory_path");

    VMObjectPtr apply() const override {
        try {
            auto p = fs::temp_directory_path();

            return path_to_object(p);
        } catch (const fs::filesystem_error& e) {
            throw error_to_object(e);
        } 
    }
};

// FS.is_block_file - checks whether the given path refers to block device
class IsBlockFile: public Monadic {
public:
    MONADIC_PREAMBLE(IsBlockFile, FS_STRING, "is_block_file");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_block_file(p0);

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.is_character_file - checks whether the given path refers to a character device
class IsCharacterFile: public Monadic {
public:
    MONADIC_PREAMBLE(IsCharacterFile, FS_STRING, "is_character_file");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_character_file(p0);

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.is_directory - checks whether the given path refers to a directory
class IsDirectory: public Monadic {
public:
    MONADIC_PREAMBLE(IsDirectory, FS_STRING, "is_directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_directory(p0);

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.is_empty - checks whether the given path refers to an empty file or directory
class IsEmpty: public Monadic {
public:
    MONADIC_PREAMBLE(IsEmpty, FS_STRING, "is_empty");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_empty(p0);

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.is_fifo - checks whether the given path refers to a named pipe
class IsFifo: public Monadic {
public:
    MONADIC_PREAMBLE(IsFifo, FS_STRING, "is_fifo");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_fifo(p0);

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.is_other - checks whether the argument refers to an other file
class IsOther: public Monadic {
public:
    MONADIC_PREAMBLE(IsOther, FS_STRING, "is_other");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_other(p0);

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.is_regular_file - checks whether the argument refers to a regular file
class IsRegularFile: public Monadic {
public:
    MONADIC_PREAMBLE(IsRegularFile, FS_STRING, "is_regular_file");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_regular_file(p0);

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};

// FS.is_socket - checks whether the argument refers to a named IPC socket
class IsSocket: public Monadic {
public:
    MONADIC_PREAMBLE(IsSocket, FS_STRING, "is_socket");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_socket(p0);

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.is_symlink - checks whether the argument refers to a symbolic link
class IsSymlink: public Monadic {
public:
    MONADIC_PREAMBLE(IsSymlink, FS_STRING, "is_symlink");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
            try {
                auto p0 = object_to_path(arg0);

                auto b = fs::is_symlink(p0);

                return bool_to_object(machine(), b);
            } catch (const fs::filesystem_error& e) {
                throw error_to_object(e);
            } 
        } else {
            return nullptr;
        }
    }
};


// FS.directory - lists the content of a directory
class Directory: public Monadic {
public:
    MONADIC_PREAMBLE(Directory, FS_STRING, "directory");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (arg0->tag() == VM_OBJECT_TEXT) {
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
            return nullptr;
        }
    }
};


// FS.status_known - checks whether file status is known XXX

extern "C" std::vector<UnicodeString> egel_imports() {
    return std::vector<UnicodeString>();
}

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

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
