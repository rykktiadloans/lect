"use strict"; // plsssssssxxs
let direction = annotationsJSON.dir === undefined ? "UD" : annotationsJSON.dir;
let shake = annotationsJSON.shake === undefined ? "roots" : annotationsJSON.shake;
let levelSeparation = 200;
let nodeSpacing = 130;


if(direction === "UD" || direction === "DU") {
    levelSeparation = 120;
    nodeSpacing = 200;
}

let nodes = [];

for (let el of annotationsJSON.code_annotations) {
    nodes.push({
        id: el.id,
        label: el.id + "\n" + el.title,
        color: {
            border: '#733e0e',
            background: '#ff9635',
            highlight: {
                border: '#73190e',
                background: '#da3521'
            },
            hover: {
                border: '#73310e',
                background: '#da4d21'
            }
        },
    });
}

for (let el of annotationsJSON.text_annotations) {
    nodes.push({ id: el.id, label: el.id + "\n" + el.title });
}


let edges = [];

for (let el of annotationsJSON.text_annotations) {
    for (let ref of el.references) {
        edges.push({
            from: el.id,
            to: ref,
        });
    }
}

let container = document.querySelector("#network");

let data = {
    nodes: nodes,
    edges: edges
};

let options = {
    layout: {
        randomSeed: undefined,
        improvedLayout: true,
        clusterThreshold: 150,
        hierarchical: {
            enabled: true,
            levelSeparation: levelSeparation, // 120 vert 170 hor
            nodeSpacing: nodeSpacing,     // 200 vert 100 hor
            treeSpacing: 300,
            blockShifting: true,
            edgeMinimization: true,
            parentCentralization: true,
            direction: direction,        // UD, DU, LR, RL
            sortMethod: 'directed',  // hubsize, directed
            shakeTowards: shake // roots, leaves
        }
    },
    edges: {
        arrows: {
            to: true
        },
        shadow: {
            enabled: true
        }
    },
    nodes: {
        shape: "box",
        widthConstraint: true,
        color: {
            border: '#116E00',
            background: '#4CC835',
            highlight: {
                border: '#009c33',
                background: '#06be37'
            },
            hover: {
                border: '#0fb217',
                background: '#3ada42'
            }
        },
        shadow: {
            enabled: true
        }
    },
    physics: false,
    interaction: {
        hover: true,
        //selectable: false
    }

    /*
    physics: {
        hierarchicalRepulsion: {
            avoidOverlap: 1,
            springConstant: 0.001
        },
    },
    */
};

let network = new vis.Network(container, data, options);

let viewer = document.querySelector("#viewer");
let control = document.querySelector("#control");
let selectedNode = "";
let clustered = false;

function onRefClick(ref) {
    displayNode(ref);
}

function onNodeClick(event) {
    let nodes = event.nodes;
    if (nodes.length === 0) {
        return;
    }
    if (nodes.length > 1) {
        return;
    }
    selectNode(nodes[0]);

}

function updateControl() {
    control.innerHTML = "";
    if(selectedNode === "" || selectedNode.includes("cluster:")) {
        control.innerHTML = "<p>Select cell to continue</p>";
        return;
    }
    console.log(selectedNode);
    control.innerHTML = `<p>Selected node with ID <b>$${selectedNode}</b></p>`;
    control.innerHTML += `<button onclick="displayNode('${selectedNode}')">Display content</button>`
    if(!clustered) {
        control.innerHTML += `<button onclick="clusterUnconnectedTo('${selectedNode}')">Hide unconnected</button>`;

    }
    else {
        control.innerHTML += `<button onclick="uncluster()">Reveal all</button>`;
    }

}

function selectNode(nodeId) {
    selectedNode = nodeId;
    updateControl();

}

function displayNode(nodeId) {
    let isCode = false;
    let annotation = annotationsJSON.text_annotations
        .find((annotation) => nodeId === annotation.id);
    if (annotation === undefined) {
        annotation = annotationsJSON.code_annotations
            .find((annotation) => nodeId === annotation.id);
        isCode = true;
    }
    viewer.style.display = "flex";
    viewer.querySelector("h4").textContent = annotation.id;
    viewer.querySelector("h1").textContent = annotation.title;
    viewer.querySelector(".file")?.remove();
    let content = viewer.querySelector(".content");
    content.innerHTML = "";
    let split = annotation.content.split("\n");
    for(let line of split) {
        if(!isCode) {
            let refs = annotation.references;
            for(let ref of refs) {
                line = line.replace("$" + ref, 
                    `<a href="#${ref}" onclick="onRefClick('${ref}')">$${ref}</a>`);
            }

        }
        let pos = 0;
        for(let char of line) {
            if(char != " ") {
                break;
            }
            pos++;
        }
        for(let i = 0; i < pos; i++) {
            content.innerHTML += "&#x00A0";
        }
        content.innerHTML += line + "<br/>";

    }
    if(isCode) {
        content.style.color = "white";
        content.style.backgroundColor = "rgb(38, 38, 38)";
        content.style.overflowX = "scroll";
        content.style.whiteSpace = "nowrap";
        let file = document.createElement("p");
        file.classList.add("file");
        file.textContent = annotation.file + ":" + annotation.line;
        content.before(file);
    }
    else {
        content.style.color = "black";
        content.style.backgroundColor = "white";
        content.style.overflowX = "hidden";
        content.style.whiteSpace = "normal";
    }
}

function clusterUnconnectedTo(nodeId) {
    clustered = true;
    network.clustering.cluster({
        joinCondition: function(nodeOptions) {
            let annotation = annotationsJSON.text_annotations.find((el) => el.id === nodeOptions.id);
            if (annotation === undefined) {
                annotation = annotationsJSON.code_annotations.find((el) => el.id === nodeOptions.id);
            }
            return annotation["connected_to"].indexOf(nodeId) == -1;
        }
    });
    updateControl();
}

function uncluster() {
    clustered = false;
    for(let el of network.body.nodeIndices) {
        if(network.isCluster(el)) {
            network.openCluster(el);
        }
    }
    updateControl();

}

network.on("click", onNodeClick);

document.querySelector(".close").onclick = () => {
    viewer.style.display = "none";
}
