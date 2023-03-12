#ifndef SRC_NODE_H
#define SRC_NODE_H

#include <vector>
#include <string>
#include <sys/types.h>

#include <string>

class Node {
public:
    virtual ~Node() {}
    virtual std::string getName() const = 0;
    virtual bool isDirectory() const = 0;
    virtual bool isFile() const = 0;

    // TODO add more methods - get size, get permissions, time of last modification, etc.
};

// TODO: Implement concrete classes for Directory and File nodes

class DirectoryNode : public Node {
public:
    virtual ~DirectoryNode() {}

    virtual std::string getName() const override;
    virtual bool isDirectory() const override;
    virtual bool isFile() const override;

    // ...
};

class FileNode : public Node {
public:
    virtual ~FileNode() {}

    virtual std::string getName() const override;
    virtual bool isDirectory() const override;
    virtual bool isFile() const override;

    // ...
};

#endif //SRC_NODE_H
