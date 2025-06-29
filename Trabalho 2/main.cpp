#define FUSE_USE_VERSION 35

#include <fuse3/fuse.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>

#define REAL_DIR "/home/vinicius_castro_filaretti/MEGA/Faculdade 5 Periodo/Sistemas Operacionais/Códigos/Trabalho 2/pasta_real"
#define LOG_FILE "/tmp/fuse_log.txt"

// Função para escrever log com filtragem de getattr desnecessário
void escrever_log(const std::string& operacao, const std::string& caminho) {
    if (operacao == "getattr" &&
        (caminho == "/" || caminho == "/.xdg-volume-info" || caminho == "/autorun.inf")) {
        return; // Ignora essas entradas
    }
    std::ofstream log(LOG_FILE, std::ios::app);
    if (log.is_open()) {
        time_t agora = time(nullptr);
        char buffer[64];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&agora));
        log << "[" << buffer << "] " << operacao << ": " << caminho << "\n";
    } else {
        std::cerr << "Erro ao abrir log: " << LOG_FILE << " errno: " << errno << std::endl;
    }
}

// Monta o caminho completo do arquivo na pasta real
std::string montar_caminho(const char* path) {
    std::string fullpath = std::string(REAL_DIR) + path;
    return fullpath;
}

// Implementação de getattr
static int fs_getattr(const char* path, struct stat* stbuf, struct fuse_file_info* fi) {
    (void)fi;
    std::string fullpath = montar_caminho(path);
    escrever_log("getattr", path);
    int res = lstat(fullpath.c_str(), stbuf);
    if (res == -1) {
        escrever_log("getattr_error", std::string(path) + " errno: " + std::to_string(errno));
        return -errno;
    }
    if (S_ISDIR(stbuf->st_mode)) {
        stbuf->st_mode |= 0755;
    } else {
        stbuf->st_mode |= 0644;
    }
    return 0;
}

// Implementação de readdir
static int fs_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info* fi,
                      enum fuse_readdir_flags flags) {
    (void)offset;
    (void)fi;
    (void)flags;

    std::string fullpath = montar_caminho(path);
    escrever_log("readdir", path);
    DIR* dp = opendir(fullpath.c_str());
    if (!dp) {
        escrever_log("readdir_error", std::string(path) + " errno: " + std::to_string(errno));
        return -errno;
    }

    struct dirent* de;
    while ((de = readdir(dp)) != nullptr) {
        struct stat st {};
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0, static_cast<fuse_fill_dir_flags>(0)) == 1) {
            break;
        }
    }

    closedir(dp);
    return 0;
}

// Implementação de open
static int fs_open(const char* path, struct fuse_file_info* fi) {
    std::string fullpath = montar_caminho(path);
    escrever_log("open", path);
    int fd = open(fullpath.c_str(), fi->flags);
    if (fd == -1) {
        escrever_log("open_error", std::string(path) + " errno: " + std::to_string(errno));
        return -errno;
    }
    close(fd);
    return 0;
}

// Implementação de read
static int fs_read(const char* path, char* buf, size_t size, off_t offset,
                   struct fuse_file_info* fi) {
    (void)fi;
    std::string fullpath = montar_caminho(path);
    escrever_log("read", path);

    int fd = open(fullpath.c_str(), O_RDONLY);
    if (fd == -1) {
        escrever_log("read_error", std::string(path) + " errno: " + std::to_string(errno));
        return -errno;
    }

    int res = pread(fd, buf, size, offset);
    if (res == -1) {
        escrever_log("read_error", std::string(path) + " errno: " + std::to_string(errno));
        close(fd);
        return -errno;
    }

    close(fd);
    return res;
}

// Implementação de write
static int fs_write(const char* path, const char* buf, size_t size,
                    off_t offset, struct fuse_file_info* fi) {
    (void)fi;
    std::string fullpath = montar_caminho(path);
    escrever_log("write", path);

    int fd = open(fullpath.c_str(), O_WRONLY);
    if (fd == -1) {
        escrever_log("write_error", std::string(path) + " errno: " + std::to_string(errno));
        return -errno;
    }

    int res = pwrite(fd, buf, size, offset);
    if (res == -1) {
        escrever_log("write_error", std::string(path) + " errno: " + std::to_string(errno));
        close(fd);
        return -errno;
    }

    close(fd);
    escrever_log("write_success", std::string(path) + " bytes_written: " + std::to_string(res));
    return res;
}

// Implementação de create
static int fs_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    std::string fullpath = montar_caminho(path);
    escrever_log("create", path);

    int fd = creat(fullpath.c_str(), mode);
    if (fd == -1) {
        escrever_log("create_error", std::string(path) + " errno: " + std::to_string(errno));
        return -errno;
    }

    fi->fh = fd;
    close(fd);
    return 0;
}

static int fs_rename(const char* from, const char* to, unsigned int flags) {
    escrever_log("rename", std::string(from) + " -> " + std::string(to));

#if defined(RENAME_NOREPLACE) // Se o sistema suportar o flag
    if (flags != 0) {
        return -EINVAL; // Só suportamos flags == 0 por simplicidade
    }
#endif

    std::string path_from = montar_caminho(from);
    std::string path_to = montar_caminho(to);

    int res = rename(path_from.c_str(), path_to.c_str());
    if (res == -1) {
        escrever_log("rename_error", std::string(from) + " -> " + std::string(to) + " errno: " + std::to_string(errno));
        return -errno;
    }

    return 0;
}

// Definir as operações do FUSE
static struct fuse_operations operacoes;

// Função principal
int main(int argc, char* argv[]) {
    operacoes.getattr = fs_getattr;
    operacoes.readdir = fs_readdir;
    operacoes.open = fs_open;
    operacoes.read = fs_read;
    operacoes.write = fs_write;
    operacoes.create = fs_create;
    operacoes.rename = fs_rename;

    std::cerr << "Iniciando FUSE com REAL_DIR: " << REAL_DIR << std::endl;
    int res = fuse_main(argc, argv, &operacoes, nullptr);
    std::cerr << "fuse_main retornou: " << res << std::endl;
    return res;
}