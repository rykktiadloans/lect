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

nodes.forEach(node => {
    node.sortkey = "0";
});

nodes.forEach(node => {
    edges.forEach(edge => {
        if (edge.to == node.id) {
            var parent = nodes.find(e => e.id == edge.from);
            node.sortkey = parent.sortkey + "." + node.id;
        }
    })
});

nodes = nodes.sort((a, b) => {
    if (a.sortkey > b.sortkey) return 1;
    if (a.sortkey < b.sortkey) return -1;
    return 0;
});

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
            levelSeparation: 120,
            nodeSpacing: 200,
            treeSpacing: 300,
            blockShifting: true,
            edgeMinimization: true,
            parentCentralization: false,
            direction: 'UD',        // UD, DU, LR, RL
            sortMethod: 'directed',  // hubsize, directed
            shakeTowards: 'leaves'  // roots, leaves
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

function onNodeClick(event) {
    network.unselectAll();
    let nodes = event.nodes;
    if (nodes.length === 0) {
        return;
    }
    if (nodes.length > 1) {
        return;
    }
    let isCode = false;
    let nodeId = nodes[0];

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
    let content = viewer.querySelector(".content");
    content.innerHTML = "";
    for(let line of annotation.content.split("\n")) {
        let pos = 0;
        for(let char of line) {
            if(char != " ") {
                break;
            }
            pos++;
        }
        console.log(pos, line);
        for(let i = 0; i < pos; i++) {
            content.innerHTML += "&#x00A0";
        }
        content.appendChild(document.createTextNode(line));
        content.append(document.createElement("br"));

    }
    if(isCode) {
        content.style.color = "white";
        content.style.backgroundColor = "rgb(38, 38, 38)";
        content.style.overflowX = "scroll";
        content.style.whiteSpace = "nowrap";
        let file = document.createElement("p");
        file.textContent = annotation.file;
        content.before(file);
    }
    else {
        content.style.color = "black";
        content.style.backgroundColor = "white";
        content.style.overflowX = "hidden";
        content.style.whiteSpace = "normal";
    }
}

network.on("click", onNodeClick);

document.querySelector(".close").onclick = () => {
    viewer.style.display = "none";
}
