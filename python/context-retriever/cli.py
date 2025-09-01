import os
import json
import questionary
import requests
from io import BytesIO
import zipfile
from pathlib import Path

from embeddings.embed import embed_context  # The function we wrote in embed.py

GITHUB_API_URL = "https://api.github.com"
ORGANIZATION = "******" # Replace with your organization
CONTEXTS_JSON = "contexts.json"

def load_contexts():
    if os.path.exists(CONTEXTS_JSON):
        with open(CONTEXTS_JSON, "r", encoding="utf-8") as f:
            return json.load(f)
    return {}

def save_contexts(ctx):
    with open(CONTEXTS_JSON, "w", encoding="utf-8") as f:
        json.dump(ctx, f, ensure_ascii=False, indent=2)

def get_github_token():
    token = os.getenv("GITHUB_TOKEN")
    if not token:
        raise ValueError("GITHUB_TOKEN not set.")
    return token

def get_headers():
    return {
        "Authorization": f"token {get_github_token()}",
        "Accept": "application/vnd.github.v3+json"
    }

def search_repos(organization, query):
    """Return up to 20 repos matching `query` in the org."""
    headers = get_headers()
    params = {
        "q": f"{query} org:{organization}",
        "per_page": "20"
    }
    url = f"{GITHUB_API_URL}/search/repositories"
    resp = requests.get(url, headers=headers, params=params)
    if resp.status_code == 200:
        data = resp.json()
        return data.get("items", [])
    else:
        print(f"Error searching: {resp.status_code} {resp.reason}")
        return []

def fetch_repo_zip(repo_full_name, context_name):
    """
    Download the repo as a zip to ./repos/<context_name>/<repo_name>.
    Return the path we extracted to, or None if fail.
    """
    print(f"Fetching: {repo_full_name} for context '{context_name}'")
    headers = get_headers()
    zip_url = f"{GITHUB_API_URL}/repos/{repo_full_name}/zipball"
    r = requests.get(zip_url, headers=headers, stream=True)
    if r.status_code == 200:
        repo_name = repo_full_name.split("/",1)[1]
        target_dir = Path("repos") / context_name / repo_name
        target_dir.mkdir(parents=True, exist_ok=True)

        zip_data = BytesIO(r.content)
        with zipfile.ZipFile(zip_data, 'r') as zip_ref:
            zip_ref.extractall(str(target_dir))

        print(f"Extracted {repo_full_name} -> {target_dir}")
        return target_dir
    else:
        print(f"Failed to fetch {repo_full_name}: {r.status_code} {r.reason}")
        return None

def main():
    contexts = load_contexts()

    while True:
        action = questionary.select(
            "Main Menu: Choose an action",
            choices=[
                "View existing contexts",
                "Create / update context with new repos",
                "Exit",
            ],
        ).ask()

        if action == "View existing contexts":
            if not contexts:
                print("No contexts exist.")
            else:
                for ctx, repos in contexts.items():
                    print(f"\nContext: {ctx}")
                    for r in repos:
                        print(f"  - {r}")

        elif action == "Create / update context with new repos":
            query = questionary.text("Search term (e.g. 'query')?").ask()
            results = search_repos(ORGANIZATION, query)
            if not results:
                print("No repos found.")
                continue

            label_map = {}
            for r in results:
                label = f"{r['full_name']} (★{r.get('stargazers_count',0)})"
                label_map[label] = r

            selected_labels = questionary.checkbox(
                "Select repos to embed:",
                choices=list(label_map.keys())
            ).ask()
            if not selected_labels:
                print("No repos selected.")
                continue

            existing_names = list(contexts.keys())
            new_or_existing = questionary.select(
                "Pick or create a context name:",
                choices=existing_names + ["Create new context"]
            ).ask()

            if new_or_existing == "Create new context":
                context_name = questionary.text("Enter context name (index)").ask()
                if not context_name.strip():
                    print("Invalid context name.")
                    continue
            else:
                context_name = new_or_existing

            # Ask if user wants to embed now
            confirm_embed = questionary.confirm(
                f"Embed {len(selected_labels)} repos into context '{context_name}' now?"
            ).ask()
            if not confirm_embed:
                print("Skipping embed.")
                continue

            # For each selected repo:
            # 1) Download if not already in contexts
            # 2) embed_context(context_name, that path)
            newly_embedded = []
            for lbl in selected_labels:
                r = label_map[lbl]
                full_name = r["full_name"]
                # if we've already embedded this repo in that context, skip
                already_in_context = context_name in contexts and (full_name in contexts[context_name])
                if already_in_context:
                    print(f"Repo {full_name} is already in context {context_name}, skipping download.")
                    continue

                # fetch repo zip => repos/<context_name>/<repo_name>
                extracted_path = fetch_repo_zip(full_name, context_name)
                if not extracted_path:
                    print("Download failed, skipping embed.")
                    continue

                # embed
                from embeddings.embed import embed_context
                embed_context(context_name, str(extracted_path))

                # add to contexts
                if context_name not in contexts:
                    contexts[context_name] = []
                contexts[context_name].append(full_name)
                newly_embedded.append(full_name)

            # save contexts
            if newly_embedded:
                save_contexts(contexts)
                print(f"Embedded {len(newly_embedded)} repos in context '{context_name}'. Updated contexts saved.")

        elif action == "Exit":
            print("Goodbye.")
            break

if __name__ == "__main__":
    main()
