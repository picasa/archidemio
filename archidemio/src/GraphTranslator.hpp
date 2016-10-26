/*
 * Copyright (C) 2011 - INRA
 * Gauthier Quesnel <quesnel@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * The Software is provided "as is", without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. In no event shall the
 * authors or copyright holders be liable for any claim, damages or other
 * liability, whether in an action of contract, tort or otherwise, arising
 * from, out of or in connection with the software or the use or other dealings
 * in the Software.
 */

#ifndef ARCHIDEMIO_DYNAMIC_GRAPH_TRANSLATOR_HPP
#define ARCHIDEMIO_DYNAMIC_GRAPH_TRANSLATOR_HPP

#include <vle/devs/Executive.hpp>
#include <vle/value/Map.hpp>
#include <Eigen/Core>
#include <string>

enum ConnectionTypeOptions
{
    CONNECTION_TYPE_IN_NAMED, /**< A port \e in is add to the destination
                                model, a port named by the source model
                                is added to the source model. */
    CONNECTION_TYPE_NAMED_OUT, /**< A port named by the source model is added
                                 to the destination model, a port \e out is
                                 added to the source model. */
    CONNECTION_TYPE_IN_OUT, /**< A port \e in is add to the destination model,
                              a port \e out is added to the destination
                              model. */
    CONNECTION_TYPE_NAMED /**< A port named by the source model name is added
                            to the destination model, a port named by the
                            destination model name is added to the source
                            model. */
};

struct Connection
{
    Connection();

    Connection(const Connection& other);

    Connection(const std::string& modelSource,
               const std::string& portSource,
               const std::string& modelDestination,
               const std::string& portDestination);

    ~Connection();

    Connection& operator=(const Connection& other);

    std::string modelSource;
    std::string portSource;
    std::string modelDestination;
    std::string portDestination;
};

typedef std::vector < std::string > ModelNameList;

typedef std::vector < Connection > ConnectionList;

/**
 * @todo
 * @code
 * DynamicGraphTranslator tr(buffer);
 * tr.build(this);
 *
 * DynamicGraphTranslator tr;
 * tr.setPrefix("node");
 * tr.setDefaultClassName("default");
 * tr.setClassName(0, "special_A");
 * tr.setClassName(1, "special_B");
 *
 * Eigen::MatrixXd m = tr.getMatrix();
 * m.setZero(3, 3);
 * m()(0, 1) = 1.0;
 * m()(0, 2) = 1.0;
 * m()(1, 2) = 1.0;
 * tr.setMatrix(m);
 *
 * tr.build(this); // instantiate the models from classes.
 *
 * [...] // in another transition
 *
 * Eigen::MatrixXd& m = tr.getMatrix();
 * m(0, 1) = 0.0;
 * tr.build(this); // delete the connection between `node-O' and `node-1'.
 * @endcode
 */
class DynamicGraphTranslator
{
public:
    explicit DynamicGraphTranslator(const vle::value::Map& buffer);

    ~DynamicGraphTranslator();

    /**
     * Get the number of node.
     *
     * @return Number of node.
     */
    int getNumber() const;

    /**
     * Get an input/output access to the adjacency matrix.
     *
     * @return A reference to the adjacency matrix.
     */
    Eigen::MatrixXd getMatrix() const;

    /**
     * Assign a new adjacency matrix.
     *
     * @param matrix The new adjacency matrix.
     *
     * @return A reference to the adjacency matrix.
     */
    void setMatrix(const Eigen::MatrixXd& matrix);

    /**
     * Assign a new connection type.
     *
     * @param type The type of connection.
     */
    void setConnectionType(ConnectionTypeOptions type);

    /**
     * Get the type of connection assigned to the GraphTranslator.
     *
     * @return The type of connection.
     */
    ConnectionTypeOptions getConnectionType() const;

    /**
     * Assign a new prefix for all newly allocated DEVS model.
     *
     * @param prefix The prefix can not be empty.
     */
    void setPrefix(const std::string& prefix);

    /**
     * Get the prefix of all newly allocated DEVS model.
     *
     * @return The prefix can not be empty.
     */
    std::string getPrefix() const;

    /**
     * Assign a new default class for all nodes where a class name is not
     * specified.
     *
     * @param name The default class name.
     */
    void setDefaultClassName(const std::string& name);

    /**
     * Get the default class name assigned to all node where a class name is
     * node specified.
     *
     * @return The default class name.
     */
    std::string getDefaultClassName() const;

    /**
     * Build the DEVS world.
     *
     * All the node are translated into devs::AtomicModel or devs::CoupledModel
     * using the class definition.
     *
     * @todo Check if connection exists / if we need to delete conenction
     *
     * @param model A pointer to the DEVS world.
     */
    void build(vle::devs::Executive* model);

    /**
     * @brief Get the list of new models between two calls to the member
     * function \e build().
     *
     * @param list
     */
    void getNewModel(ModelNameList* list) const;

    /**
     * @brief Get the list of deleted models between two calls to the member
     * function \e build().
     *
     * @param list
     */
    void getDeletedModel(ModelNameList* list) const;

    /**
     * @brief Get the list of new connection between two call to the member
     * function \e build().
     *
     * @param list
     */
    void getNewConnection(ConnectionList* list) const;

    /**
     * @brief Get the list of deleted connections between two call to the
     * member function \e build().
     *
     * @param list
     */
    void getDeletedConnection(ConnectionList* list) const;

private:
    DynamicGraphTranslator(const DynamicGraphTranslator&);
    DynamicGraphTranslator& operator=(const DynamicGraphTranslator&);

    class Pimpl;
    Pimpl* mPimpl;
};

#endif
