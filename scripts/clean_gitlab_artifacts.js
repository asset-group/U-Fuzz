#!/usr/bin/env node

const fetch = require('node-fetch');

//Go to: https://gitlab.com/profile/personal_access_tokens
const API_KEY = process.env.GITLAB_TOKEN;

//You can find project id inside the "General project settings" tab
const PROJECT_ID = 11253692;
const PROJECT_URL = "https://gitlab.com/api/v4/projects/" + PROJECT_ID + "/"


async function sendApiRequest(url, options = {}) {
    if (!options.headers)
        options.headers = {};
    options.headers["PRIVATE-TOKEN"] = API_KEY;

    return fetch(url, options);
}

async function main() {
    let jobs = [];
    console.log("----------- Cleaning old artifacts -----------")
    console.log("Getting jobs ids")
    for (let i = 0, currentJobs = []; i == 0 || currentJobs.length > 0; i++) {
        console.log("Getting page " + (i + 1) + "...");
        currentJobs = await sendApiRequest(
            PROJECT_URL + "jobs/?per_page=100&page=" + (i + 1)
        ).then(e => e.json());
        console.log("--> Got " + currentJobs.length + " jobs");
        jobs = jobs.concat(currentJobs);
    }

    console.log("Got a total of " + jobs.length + " jobs")

    //skip jobs without artifacts
    jobs = jobs.filter(e => e.artifacts_file);

    //keep the latest build.
    jobs.shift();
    console.log("Jobs to clean-up: " + jobs.length)

    for (let job of jobs) {
        console.log("Deleting artifacts with job id=" + job.id)
        res = await sendApiRequest(
            PROJECT_URL + "jobs/" + job.id + "/artifacts", { method: "DELETE" }
        );
        console.log("--> Cleanud-up job " + job.id)
    }
}

main();