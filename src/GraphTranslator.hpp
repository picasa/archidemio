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

#ifndef VLE_TRANSLATOR_DYNAMIC_GRAPH_TRANSLATOR_HPP
#define VLE_TRANSLATOR_DYNAMIC_GRAPH_TRANSLATOR_HPP

#include <vle/devs/Executive.hpp>
#include <vle/value/Tuple.hpp>
#include <vle/value/Map.hpp>
#include <Eigen/Core>
#include <string>

enum ConnectionTypeOptions {
    CONNECTION_TYPE_IN_NAMED,
    CONNECTION_TYPE_NAMED_OUT,
    CONNECTION_TYPE_IN_OUT,
    CONNECTION_TYPE_NAMED
};

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
 * tr.getMatrix().setZero(3, 3);
 * tr.getMatrix()(0, 1) = 1.0;
 * tr.getMatrix()(0, 2) = 1.0;
 * tr.getMatrix()(1, 2) = 1.0;
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
    Eigen::MatrixXd& getMatrix() const;

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
     * Build the DEVS world. All the node are translated into devs::AtomicModel
     * or devs::CoupledModel using the class definition.
     *
     * @todo Check if connection exists / if we need to delete conenction
     *
     * @param model A pointer to the DEVS world.
     */
    void build(vle::devs::Executive* model);

private:
    DynamicGraphTranslator(const DynamicGraphTranslator&);
    DynamicGraphTranslator& operator=(const DynamicGraphTranslator&);

    class Pimpl;
    Pimpl* mPimpl;
};

#endif
